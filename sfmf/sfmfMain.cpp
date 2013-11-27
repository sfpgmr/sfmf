#include "pch.h"
#include "sfmfMain.h"
#include "Common\DirectXHelper.h"

using namespace sfmf;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// アプリケーションの読み込み時にアプリケーション資産を読み込んで初期化します。
sfmfMain::sfmfMain(const std::shared_ptr<DX::DeviceResources>& deviceResources,DirectXPage^ page) :
m_deviceResources(deviceResources), m_pointerLocationX(0.0f), m_page(page)
{
	// デバイスが失われたときや再作成されたときに通知を受けるように登録します
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: これをアプリのコンテンツの初期化で置き換えます。
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: 既定の可変タイムステップ モード以外のモードが必要な場合は、タイマー設定を変更してください。
	// 例: 60 FPS 固定タイムステップ更新ロジックでは、次を呼び出します:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
	m_mf.reset(new sf::AutoMF);



}

sfmfMain::~sfmfMain()
{
	// デバイスの通知を登録解除しています
	m_deviceResources->RegisterDeviceNotify(nullptr);
	m_mf.reset();
}

// ウィンドウのサイズが変更される (デバイスの方向が変更されるなど) 場合に、 アプリケーションの状態を更新します。
void sfmfMain::CreateWindowSizeDependentResources() 
{
	// TODO: これをアプリのコンテンツのサイズに依存する初期化で置き換えます。
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void sfmfMain::StartRenderLoop()
{
	// アニメーション レンダリング ループが既に実行中の場合は、別のスレッドを開始しないでください。
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// バックグラウンド スレッドで実行するタスクを作成します。
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// 更新されたフレームを計算し、垂直帰線消去期間ごとに 1 つレンダリングします。
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// 優先順の高い専用のバックグラウンド スレッドでタスクを実行します。
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void sfmfMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// アプリケーション状態をフレームごとに 1 回更新します。
void sfmfMain::Update() 
{
	ProcessInput();

	// シーン オブジェクトを更新します。
	m_timer.Tick([&]()
	{
		// TODO: これをアプリのコンテンツの更新関数で置き換えます。
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// ゲーム状態を更新する前にユーザーからのすべての入力を処理します
void sfmfMain::ProcessInput()
{
	// TODO: フレームごとの入力処理方法をここに入力します。
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// 現在のアプリケーション状態に応じて現在のフレームをレンダリングします。
// フレームがレンダリングされ、表示準備が完了すると、true を返します。
bool sfmfMain::Render() 
{
	// 初回更新前にレンダリングは行わないようにしてください。
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// ビューポートをリセットして全画面をターゲットとします。
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// レンダリング ターゲットを画面にリセットします。
	//ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	//context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// バック バッファーと深度ステンシル ビューをクリアします。
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// シーン オブジェクトをレンダリングします。
	// TODO: これをアプリのコンテンツのレンダリング関数で置き換えます。
//	m_sceneRenderer->RenderToTexture();
//	m_fpsTextRenderer->Render(m_sceneRenderer->GetvideoTextureBitmap());
	m_sceneRenderer->Render();

	return true;
}

// デバイス リソースを解放する必要が生じたことをレンダラーに通知します。
void sfmfMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// デバイス リソースの再作成が可能になったことをレンダラーに通知します。
void sfmfMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void sfmfMain::OpenFile()
{

	auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	openPicker->FileTypeFilter->Append(L".wav");
	openPicker->FileTypeFilter->Append(L".mp4");
	create_task(openPicker->PickSingleFileAsync()).then([this](Windows::Storage::StorageFile^ file)
	{
		if (file != nullptr){
			sf::dout(file->DisplayName);
			sf::dout(file->Path);
			sf::dout(file->IsAvailable.ToString());
			//m_audioReader = ref new sf::AudioReader(file->Path);

			create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read)).then([this, file](Windows::Storage::Streams::IRandomAccessStream^ stream)
			{
				create_async([this,stream](){
					IMFSamplePtr sample;
					m_audioReader = ref new sf::AudioReader(stream);
					UINT status = 0;
					while (status != MF_SOURCE_READERF_ENDOFSTREAM)
					{
						status = m_audioReader->ReadSample(sample);
						sf::dout(boost::wformat(L"%10x \n") % m_audioReader->SampleTime);
					}
				});
			});
			
			/*create_task(Windows::Storage::KnownFolders::MusicLibrary->CreateFileAsync
				(L"test.m4v", Windows::Storage::CreationCollisionOption::ReplaceExisting))
				.then([this](Windows::Storage::StorageFile^ f){
				sf::dout(f->DisplayName);
				sf::dout(f->Path);
				sf::dout(f->IsAvailable.ToString());
				create_task(f->OpenAsync()).then([this, f](task<Windows::Storage::Streams::IRandomAccessStreamWithContentType^> streamTask){
					try {
						auto stream = streamTask.get();
						sf::dout(L"Stream Opened!!");
					}
					catch (Platform::Exception^ ex){
						sf::dout(boost::wformat(L"%08x %s") % ex->HResult % ex->Message->Data());
					}

				});
			});*/
			//auto stream = create_task(st->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite)).get();
				//m_videoWriter = ref new sf::VideoWriter(stream);
		}
	}
	);

}
