//
// App.xaml.h
// App クラスの宣言。
//

#pragma once

#include "App.g.h"
#include "DirectXPage.xaml.h"

namespace sfmf
{
		/// <summary>
	/// 既定の Application クラスを補完するアプリケーション固有の動作を提供します。
	/// </summary>
	ref class App sealed
	{
	public:
		App();
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object ^sender, Platform::Object ^args);
		DirectXPage^ m_directXPage;
	};
}
