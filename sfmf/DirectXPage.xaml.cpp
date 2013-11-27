//
// DirectXPage.xaml.cpp
// DirectXPage クラスの実装です。
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace sfmf;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// ページ ライフサイクルのイベント ハンドラーを登録します。
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// タッチ時のパフォーマンスが向上するように、ポインターのビジュアル フィードバックをすべて無効にします。
	auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerVisualizationSettings->IsContactFeedbackEnabled = false; 
	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

	// この時点では、デバイスにアクセスできます。
	// デバイスに依存するリソースを作成できます。
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// 独立した入力ポインター イベントを取得するために、SwapChainPanel を登録します
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// 指定したデバイスの種類が、どのスレッドで作成された場合であっても、CoreIndependentInputSource はポインター イベントを発生させます。
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// バックグラウンド スレッドでポインター イベントが発生したときに、そのイベントを登録します。
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// 入力メッセージが配信された時点で、そのメッセージの処理を開始します。
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// 優先順の高い専用のバックグラウンド スレッドでタスクを実行します。
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<sfmfMain>(new sfmfMain(m_deviceResources,this));
	m_main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// 破棄の際に、レンダリング イベントと処理イベントを停止します
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// 中断および終了イベントに対してアプリの現在の状態を保存します。
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// アプリケーションが中断された時点で、レンダリングを停止します。
	m_main->StopRenderLoop();

	// コードを入力して、アプリ状態をここに保存します。
}

// 再開イベントに対してアプリの現在の状態を読み込みます。
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// コードを入力して、アプリ状態をここに読み込みます。

	// アプリケーションが再開された時点で、レンダリングを開始します。
	m_main->StartRenderLoop();
}

// ウィンドウ イベント ハンドラー。

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation イベント ハンドラー。

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}


void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// アプリ バー ボタンがクリックされたときに呼び出されます。
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// アプリ バーは、アプリに適切である場合に使用します。アプリ バーを設計し、
	//このようにイベント ハンドラーに格納します。
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// ポインターがクリックされた時点で、ポインターの移動の追跡を開始します。
	m_main->StartTracking();
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// ポインター追跡コードを更新します。
	if (m_main->IsTracking())
	{
		m_main->TrackingUpdate(e->CurrentPoint->Position.X);
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// ポインターが解放された時点で、ポインターの移動の追跡を停止します。
	m_main->StopTracking();
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}



void sfmf::DirectXPage::openFileButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	m_main->OpenFile();
}
