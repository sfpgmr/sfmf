//
// DirectXPage.xaml.h
// DirectXPage クラスの宣言。
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "sfmfMain.h"
#include "sfmf.h"


namespace sfmf
{
	/// <summary>
	/// DirectX SwapChainPanel をホストするページ。
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);
	internal:
		property  Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel {
			Windows::UI::Xaml::Controls::SwapChainPanel^ get() { return swapChainPanel_; }
		};
		property ::Windows::UI::Xaml::Controls::MediaElement^ videoPreview {
			::Windows::UI::Xaml::Controls::MediaElement^ get() { return videoPreview_; }
		};

		property ::Windows::UI::Xaml::Controls::Button^ openFileButton {
			::Windows::UI::Xaml::Controls::Button^ get() { return openFileButton_; }
		}
    void SetProgress(Windows::Foundation::IAsyncActionWithProgress<float>^ progress);
  private:
		// XAML 低レベル レンダリング イベント ハンドラー。
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// ウィンドウ イベント ハンドラー。
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// DisplayInformation イベント ハンドラー。
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// その他のイベント ハンドラー。
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// バックグラウンド ワーカー スレッドで、独立入力を追跡します。
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

		// 独立入力の処理関数。
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		// XAML ページ背景で DirectX コンテンツを描画する場合に使用するリソース。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<sfmfMain> m_main; 
		bool m_windowVisible;
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void openFileButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    void ProgressInfo__ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void ProgressInfo__ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  };
}

