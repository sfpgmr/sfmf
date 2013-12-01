#include "pch.h"
#include "sfmfMain.h"
#include "Common\DirectXHelper.h"

using namespace sfmf;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// アプリケーションの読み込み時にアプリケーション資産を読み込んで初期化します。
sfmfMain::sfmfMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, DirectXPage^ page) :
m_deviceResources(deviceResources), m_pointerLocationX(0.0f), m_page(page)
{
  // デバイスが失われたときや再作成されたときに通知を受けるように登録します
  m_deviceResources->RegisterDeviceNotify(this);

  // TODO: これをアプリのコンテンツの初期化で置き換えます。
  m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

  m_d2dRenderer = std::unique_ptr<Direct2DRenderer>(new Direct2DRenderer(m_deviceResources));

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
    //while (action->Status == AsyncStatus::Started)
    //{
    //  {
    //    critical_section::scoped_lock lock(m_criticalSection);
    //    //     if (m_criticalSection.try_lock()){
    //    Update();
    //    if (Render())
    //    {
    //      m_deviceResources->Present();
    //    }
    //    //       m_criticalSection.unlock();
    //    //      }

    //  }
    //}
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
  //	m_d2dRenderer->Render(m_sceneRenderer->GetvideoTextureBitmap());
  m_sceneRenderer->Render();
  return true;
}

// デバイス リソースを解放する必要が生じたことをレンダラーに通知します。
void sfmfMain::OnDeviceLost()
{
  m_sceneRenderer->ReleaseDeviceDependentResources();
  m_d2dRenderer->ReleaseDeviceDependentResources();
}

// デバイス リソースの再作成が可能になったことをレンダラーに通知します。
void sfmfMain::OnDeviceRestored()
{
  m_sceneRenderer->CreateDeviceDependentResources();
  m_d2dRenderer->CreateDeviceDependentResources();
  CreateWindowSizeDependentResources();
}

ref class StreamHolder {
internal:
  Windows::Storage::Streams::IRandomAccessStream^ readStream;
  Windows::Storage::Streams::IRandomAccessStream^ writeStream;
};


void sfmfMain::OpenFile()
{
  ready_ = false;
  auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
  openPicker->FileTypeFilter->Append(L".wav");
  auto holder = ref new StreamHolder();
  // Wave ファイルを1個指定する
  create_task(openPicker->PickSingleFileAsync())
    .then([this, holder](Windows::Storage::StorageFile^ file)
  {
    if (file != nullptr){

      DOUT(file->DisplayName);
      DOUT(file->Path);
      DOUT(file->IsAvailable.ToString());
      //m_audioReader = ref new sf::AudioReader(file->Path);
      // .WAVファイルを開く
      create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read))
        .then([this, file, holder](task<Windows::Storage::Streams::IRandomAccessStream^> streamTask) {
        // AudioReaderを作成する
        holder->readStream = streamTask.get();
        m_audioReader = ref new sf::AudioReader(holder->readStream);
        // 出力ファイル(M4V)を作成する
        return create_task(Windows::Storage::KnownFolders::VideosLibrary->CreateFileAsync
          ((file->DisplayName + L".mp4"), Windows::Storage::CreationCollisionOption::ReplaceExisting));
      }).then([this, holder](task<Windows::Storage::StorageFile^> fileTask){
        // 出力ファイルを開く
        return create_task(fileTask.get()->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite));
      }).then([this, holder](task<Windows::Storage::Streams::IRandomAccessStream^> streamTask){
        // VideoWriterオブジェクトを作成する
        holder->writeStream = streamTask.get();
        m_videoWriter = ref new sf::VideoWriter(holder->writeStream);
      }).then([this, holder]()->void {
        // オーディオファイルを読み込み、m4vファイルに書き込む
          m_writeProgress = create_async([this, holder](concurrency::progress_reporter<float> reporter, concurrency::cancellation_token ct)->void {
          DWORD status = 0;
          m_videoTime = 0;
          LONGLONG readSize = 0;
          while (true)
          {
            IMFSamplePtr sample;
            // オーディオサンプルを読み込む
            status = m_audioReader->ReadSample(sample);
 
            if ((status & MF_SOURCE_READERF_ENDOFSTREAM) || ct.is_canceled() ) {
              // EOFもしくは中断したらファイナライズ処理を行う
              m_videoWriter->Finalize();
              reporter.report(100.0f);
              break;
            }
            DWORD size = 0;
            sample->GetTotalLength(&size);
            readSize += size;
            LONGLONG time;
            sample->GetSampleTime(&time);
            m_videoStepTime = time - m_videoTime;
            m_videoTime = time;

            m_videoWriter->WriteAudioSample(sample.Get());

            RenderToVideo(sample.Get());
            reporter.report(((float)readSize / (float) m_audioReader->FileSize) * 100.0f);
          }
        });
        m_page->SetProgress(m_writeProgress);
      });
    }
  });
}

void sfmfMain::RenderToVideo(IMFSample* sample)
{
  // Direct3Dでオフスクリーンにレンダリングし、そのデータを書き込む
  IMFMediaBufferPtr buffer;
  CHK(sample->GetBufferByIndex(0, &buffer));
  INT16* waveBuffer;
  const DWORD lengthTick = 44100 /* Hz */ * 2 /* CH */ * 30 /* ms */ / 1000 /* ms */;
  DWORD startPos = 0;
  CHK(buffer->Lock((BYTE**) &waveBuffer, nullptr, nullptr));
  DWORD totalLength;
  CHK(buffer->GetCurrentLength(&totalLength));
  totalLength /= 2;
  DWORD length = lengthTick;

  while (m_videoTime > m_videoWriter->VideoSampleTime)
  {
    m_sceneRenderer->Update(m_videoWriter->VideoSampleTime);
    m_d2dRenderer->Update(m_videoWriter->VideoSampleTime);
    {
      // コンテキストの競合を回避するためにロックする
      critical_section::scoped_lock lock(m_criticalSection);
      // Direct3D11によるレンダリング
      m_sceneRenderer->RenderToTexture();
      // Direct2Dによるレンダリング
      m_d2dRenderer->Render(&waveBuffer[startPos],length,m_sceneRenderer->GetvideoTextureBitmap());
      startPos += lengthTick;
      if ((totalLength - startPos) > lengthTick)
      {
        length = lengthTick;
      }
      else {
        length = totalLength - startPos;
      }
      // 描画したテクスチャデータをステージテクスチャにコピーする
      m_sceneRenderer->CopyStageTexture();
      // ビデオデータを作成し、サンプルに収める
      m_videoWriter->SetTextureToSample(m_deviceResources->GetD3DDeviceContext(), m_sceneRenderer->GetVideoStageTexture());
    }

    // ビデオデータを書き込む
    m_videoWriter->WriteVideoSample();
  }

  CHK(buffer->Unlock());
}
