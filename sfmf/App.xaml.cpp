//
// App.xaml.cpp
// App クラスの実装。
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace sfmf;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
/// <summary>
/// 単一アプリケーション オブジェクトを初期化します。これは、実行される作成したコードの
/// 最初の行であり、main() または WinMain() と論理的に等価です。
/// </summary>
App::App()
{
	InitializeComponent();
	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
	Resuming += ref new EventHandler<Object^>(this, &App::OnResuming);
}

/// <summary>
/// アプリケーションがエンド ユーザーによって正常に起動されたときに呼び出されます。他のエントリ ポイントは、
/// アプリケーションが特定のファイルを開くために呼び出されたときに
/// 検索結果やその他の情報を表示するために使用されます。
/// </summary>
/// <param name="e">起動要求とプロセスの詳細を表示します。</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
#if _DEBUG
	if (IsDebuggerPresent())
	{
		DebugSettings->EnableFrameRateCounter = true;
	}
#endif

	m_directXPage = ref new DirectXPage();

	if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
	{
		m_directXPage->LoadInternalState(ApplicationData::Current->LocalSettings->Values);
	}

	// ページを現在のウィンドウに配置し、アクティブであることを確認します。
	Window::Current->Content = m_directXPage;
	Window::Current->Activate();
}

/// <summary>
/// アプリケーションの実行が中断されたときに呼び出されます。アプリケーションの状態は、
/// アプリケーションが終了されるのか、メモリの内容がそのままで再開されるのか
/// わからない状態で保存されます。
/// </summary>
/// <param name="sender">中断要求の送信元。</param>
/// <param name="e">中断要求の詳細。</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void) sender;	// 未使用のパラメーター
	(void) e;	// 未使用のパラメーター

	m_directXPage->SaveInternalState(ApplicationData::Current->LocalSettings->Values);
}

/// <summary>
/// アプリケーションの実行が再開されたときに呼び出されます。
/// </summary>
/// <param name="sender">再開要求の送信元。</param>
/// <param name="args">再開要求の詳細。</param>
void App::OnResuming(Object ^sender, Object ^args)
{
	(void) sender; // 未使用のパラメーター
	(void) args; // 未使用のパラメーター

	m_directXPage->LoadInternalState(ApplicationData::Current->LocalSettings->Values);
}
