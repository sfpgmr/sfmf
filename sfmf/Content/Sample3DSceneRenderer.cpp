#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace sfmf;

using namespace DirectX;
using namespace Windows::Foundation;

extern const long long hnsSampleDuration;

// ファイルから頂点とピクセル シェーダーを読み込み、キューブのジオメトリをインスタンス化します。
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources),
	m_videoViewport(0.0f, 0.0f, (float) VIDEO_WIDTH, (float) VIDEO_HEIGHT)
{
	m_clearColor[0] = 0.0f;
	m_clearColor[1] = 0.0f;
	m_clearColor[2] = 0.0f;
	m_clearColor[3] = 1.0f;
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// ウィンドウのサイズが変更されたときに、ビューのパラメーターを初期化します。
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	
	//Size outputSize = m_deviceResources->GetOutputSize();
	Size outputSize = { VIDEO_WIDTH, VIDEO_HEIGHT };
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 90.0f * XM_PI / 180.0f;


	// これは、アプリケーションが縦向きビューまたはスナップ ビュー内にあるときに行うことのできる
	// 変更の簡単な例です。
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// OrientationTransform3D マトリックスは、シーンの方向を表示方向と
	// 正しく一致させるため、ここで事後乗算されます。
	// この事後乗算ステップは、スワップ チェーンのターゲット ビットマップに対して行われるすべての
	// 描画呼び出しで実行する必要があります。他のターゲットに対する呼び出しでは、
	// 適用する必要はありません。

	// このサンプルでは、行優先のマトリックスを使用した右辺座標系を使用しています。
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	//XMStoreFloat4x4(
	//	&m_constantBufferData.projection,
	//	XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	//	);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix)
		);
	// 視点は (0,0.7,1.5) の位置にあり、y 軸に沿って上方向のポイント (0,-0.1,0) を見ています。
	static const XMVECTORF32 eye = { 0.0f, 0.0f, 2.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// フレームごとに 1 回呼び出し、キューブを回転させてから、モデルおよびビューのマトリックスを計算します。
void Sample3DSceneRenderer::Update(LONGLONG stepTime)
{
	if (!m_tracking)
	{
		// 度をラジアンに変換し、秒を回転角度に変換します
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = (double)(stepTime) / 10000000.0 * (double)radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
		Rotate(radians);
	}
}

//3D キューブ モデルを、ラジアン単位で設定された大きさだけ回転させます。
void Sample3DSceneRenderer::Rotate(float radians)
{
	//更新されたモデル マトリックスをシェーダーに渡す準備をします
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// 追跡時に、出力画面の幅方向を基準としてポインターの位置を追跡することにより、3D キューブを Y 軸に沿って回転させることができます。
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
//		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		float radians = XM_2PI * 2.0f * positionX / (float)VIDEO_WIDTH;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// 頂点とピクセル シェーダーを使用して、1 つのフレームを描画します。
void Sample3DSceneRenderer::Render()
{
		// 読み込みは非同期です。読み込みが完了した後にのみ描画してください。
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// レンダー ターゲットをテクスチャに設定します。
	ID3D11RenderTargetView * targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, nullptr);

	uint32 stride = sizeof(VertexVideo);
	uint32 offset = 0;

	context->VSSetShader(m_videoVertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_videoPixelShader.Get(), nullptr, 0);

	context->IASetVertexBuffers(0, 1, m_videoVertex.GetAddressOf(), &stride, &offset);
	context->IASetInputLayout(m_videoVertexLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11SamplerState *samplers[1] = { m_videoSamplerState.Get() };
	context->PSSetSamplers(0, 1, samplers);
	ID3D11ShaderResourceView * srv[1] = { m_videoResourceView.Get()};
	context->PSSetShaderResources(0, 1, srv);
	context->RSSetState(m_videoRasterState.Get());
	context->RSSetViewports(1, &m_videoViewport);
//	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), m_clearColor);
	context->Draw(4, 0);

	srv[0] = { nullptr };
	context->PSSetShaderResources(0, 1, srv);
	targets[0] = nullptr;
	context->OMSetRenderTargets(1, targets, m_videoDepthView.Get());

}

// テクスチャーにレンダリングする。
void Sample3DSceneRenderer::RenderToTexture()
{
	// 読み込みは非同期です。読み込みが完了した後にのみ描画してください。
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// レンダー ターゲットをテクスチャに設定します。
	ID3D11RenderTargetView * targets[1] = { m_videoRenderTargetView.Get() };
	context->OMSetRenderTargets(1, targets, m_videoDepthView.Get());
	context->RSSetViewports(1, &m_videoViewport);
	float color[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
	context->ClearRenderTargetView(m_videoRenderTargetView.Get(), color);
	context->ClearDepthStencilView(m_videoDepthView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	
	//ID3D11RenderTargetView * targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	//context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());


  //	context->ClearRenderTargetView(m_videoRenderTargetView.Get(), m_clearColor);
	
	// 定数バッファーを準備して、グラフィックス デバイスに送信します。
	context->UpdateSubresource(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0
		);

	// 各頂点は、VertexPositionColor 構造体の 1 つのインスタンスです。
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // 各インデックスは、1 つの 16 ビット符号なし整数 (short) です。
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// 頂点シェーダーをアタッチします。
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	// 定数バッファーをグラフィックス デバイスに送信します。
	context->VSSetConstantBuffers(
		0,
		1,
		m_constantBuffer.GetAddressOf()
		);


	// ピクセル シェーダーをアタッチします。
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	ID3D11ShaderResourceView* srv[1] = { nullptr };
	context->PSSetShaderResources(0, 1, srv);

	// オブジェクトを描画します。
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);


	targets[0] =  nullptr ;
	context->OMSetRenderTargets(1, targets, m_videoDepthView.Get());



//	context->OMSetRenderTargets(1, nullptr, nullptr);


}

void Sample3DSceneRenderer::CopyStageTexture()
{
  // CPU読み取り可能なサーフェースにコピーする
  m_deviceResources->GetD3DDeviceContext()->CopyResource(m_videoStageTexture.Get(), m_videoTexture.Get());

}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// シェーダーを非同期で読み込みます。
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadVideoPSTask = DX::ReadDataAsync(L"VideoPixelShader.cso");
	auto loadVideoVSTask = DX::ReadDataAsync(L"VideoVertexShader.cso");

	// 頂点シェーダー ファイルを読み込んだ後、シェーダーと入力レイアウトを作成します。
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});

	// ピクセル シェーダー ファイルを読み込んだ後、シェーダーと定数バッファーを作成します。
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	auto createVideoVSTask = loadVideoVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
			&fileData[0],
			fileData.size(),
			nullptr,
			&m_videoVertexShader
			)
			);

		static D3D11_INPUT_ELEMENT_DESC VideoVertex2DLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
			VideoVertex2DLayout,
			ARRAYSIZE(VideoVertex2DLayout),
			&fileData[0],
			fileData.size(),
			&m_videoVertexLayout
			)
			);


	});

	auto createVideoPSTask = loadVideoPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
			&fileData[0],
			fileData.size(),
			nullptr,
			&m_videoPixelShader
			)
			);

	});

	// 両方のシェーダーの読み込みが完了したら、メッシュを作成します。
	auto createCubeTask = (createPSTask && createVSTask && createVideoVSTask && createVideoPSTask).then([this] () {

		// メッシュの頂点を読み込みます。各頂点には、位置と色があります。
		static const VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
				)
			);

		// メッシュのインデックスを読み込みます。インデックスの 3 つ 1 組の値のそれぞれは、次のものを表します:
		// 画面上に描画される三角形を表します。
		// たとえば、0,2,1 とは、頂点バッファーからのインデックスを意味します:
		// 0、2、1 を持つ頂点が、このメッシュの
		// 最初の三角形を構成することを意味します。
		static const unsigned short cubeIndices [] =
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
				)
			);
	});

	// キューブが読み込まれたら、オブジェクトを描画する準備が完了します。

	auto videoInitTask = createCubeTask.then([this]()
	{

		// スワップチェインのバッファを取得する
		ID3D11Texture2DPtr buffer;
		DX::ThrowIfFailed(m_deviceResources->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer)));
		D3D11_TEXTURE2D_DESC desc, bufferDesc;
		buffer->GetDesc(&bufferDesc);
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		// スワップチェインバッファの中身の複製を保存するテクスチャの作成
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = bufferDesc.Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = 0;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Width = VIDEO_WIDTH;
		desc.Height = VIDEO_HEIGHT;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &m_videoTexture));

		// 複製テクスチャー用のSRV
		CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
			m_videoTexture.Get(),
			D3D11_SRV_DIMENSION_TEXTURE2D
			);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateShaderResourceView(m_videoTexture.Get(), &shaderResourceViewDesc, &m_videoResourceView));

		// RTVの作成
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
			m_videoTexture.Get(),
			D3D11_RTV_DIMENSION_TEXTURE2D,
			bufferDesc.Format);

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateRenderTargetView(
			m_videoTexture.Get(),
			&renderTargetViewDesc,
			&m_videoRenderTargetView
			)
			);

		D3D11_RASTERIZER_DESC1 RasterizerDesc = {
			D3D11_FILL_SOLID,
			D3D11_CULL_NONE,	//ポリゴンの裏表を無くす
			FALSE,
			0,
			0.0f,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE
		};

		// 深度バッファ・ビューの作成
		// 必要な場合は 3D レンダリングで使用する深度ステンシル ビューを作成します。
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			static_cast<UINT>(VIDEO_WIDTH),
			static_cast<UINT>(VIDEO_HEIGHT),
			1, // この深度ステンシル ビューには、1 つのテクスチャしかありません。
			1, // 1 つの MIPMAP レベルを使用します。
			D3D11_BIND_DEPTH_STENCIL
			);

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&m_videoDepthTexture
			)
			);

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDepthStencilView(
			m_videoDepthTexture.Get(),
			&depthStencilViewDesc,
			&m_videoDepthView
			)
			);

		// テクスチャ・サンプラの生成
		D3D11_SAMPLER_DESC sampDesc = {};

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.MaxAnisotropy = 1;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = FLT_MAX;
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, &m_videoSamplerState));

		m_deviceResources->GetD3DDevice()->CreateRasterizerState1(&RasterizerDesc, &m_videoRasterState);

		// CPUから読み取り可能なサーフェースを生成
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, 0, &m_videoStageTexture));

		float adjust_width = (m_deviceResources->GetOutputSize().Width / m_deviceResources->GetOutputSize().Height) / (((float) VIDEO_WIDTH / (float) VIDEO_HEIGHT));
		float adjustTextureU = VIDEO_WIDTH / m_deviceResources->GetOutputSize().Width;
		float adjustTextureV = VIDEO_HEIGHT / m_deviceResources->GetOutputSize().Height;

			VertexVideo videoVerticies[4] = 
		{
      { DirectX::XMFLOAT2(-adjust_width, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
      { DirectX::XMFLOAT2(-adjust_width, 1.0f), DirectX::XMFLOAT2(0.0f, adjustTextureV) },
      { DirectX::XMFLOAT2(adjust_width, -1.0f), DirectX::XMFLOAT2(adjustTextureU, 0.0f) },
      { DirectX::XMFLOAT2(adjust_width, 1.0f), DirectX::XMFLOAT2(adjustTextureU, adjustTextureV) }
		};


		//頂点バッファ作成
		D3D11_BUFFER_DESC BufferDesc;
		BufferDesc.ByteWidth = sizeof(VertexVideo) * ARRAYSIZE(videoVerticies);
		BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = 0;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = sizeof(float);

		D3D11_SUBRESOURCE_DATA SubResourceData;
		SubResourceData.pSysMem = videoVerticies;
		SubResourceData.SysMemPitch = 0;
		SubResourceData.SysMemSlicePitch = 0;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&BufferDesc, &SubResourceData, &m_videoVertex));

		// ビデオテクスチャにDirect2DからアクセスするためのID2D1Bitmapの作成
		IDXGISurfacePtr surface;
		m_videoTexture.As(&surface);
		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
			);
		DX::ThrowIfFailed(m_deviceResources->GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(surface.Get(), bitmapProperties, &m_videoBitmap));
	
		//m_deviceResources->GetD2DDeviceContext()->Cre

		// テクスチャーをビデオストリームに保存するクラスの作成
		// m_videoWriter = ref new sf::VideoWriter(create_task(st->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite)).get());
	});


	videoInitTask.then([this]() {
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_videoVertexShader.Reset();
	m_videoPixelShader.Reset();
	m_videoVertexLayout.Reset();
}

void Sample3DSceneRenderer::WriteVideoFrame()
{
	//static int count = 0;
	//if (!count){
	//	{
	//		ID3D11Texture2DPtr pSrcBuffer;
	//		DX::ThrowIfFailed(m_deviceResources->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&pSrcBuffer)));
	//		m_deviceResources->GetD3DDeviceContext()->CopyResource(m_videoTexture.Get(), pSrcBuffer.Get());
	//	}
	//	uint32 stride = sizeof(VertexVideo);
	//	uint32 offset = 0;
	//	m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, m_videoVertex.GetAddressOf(), &stride, &offset);
	//	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_videoVertexLayout.Get());
	//	m_deviceResources->GetD3DDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	m_deviceResources->GetD3DDeviceContext()->VSSetShader(m_videoVertexShader.Get(), nullptr, 0);
	//	m_deviceResources->GetD3DDeviceContext()->PSSetShader(m_videoPixelShader.Get(), nullptr, 0);
	//	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(1, m_videoRenderTargetView.GetAddressOf(), nullptr);
	//	m_deviceResources->GetD3DDeviceContext()->PSSetSamplers(0, 1, m_videoSamplerState.GetAddressOf());
	//	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_videoSrcView.GetAddressOf());
	//	m_deviceResources->GetD3DDeviceContext()->RSSetState(m_videoRasterState.Get());
	//	m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &m_videoViewport);
	//	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(m_videoRenderTargetView.Get(), m_clearColor);
	//	m_deviceResources->GetD3DDeviceContext()->Draw(4, 0);
	//	m_deviceResources->GetD3DDeviceContext()->CopyResource(m_videoStageTexture.Get(), m_videoTexture.Get());
	//	//m_videoWriter->WriteTextureToBuffer(m_d3dContext, m_videoStageTexture);
	//}
	//else {
	//	//m_videoWriter->WriteSink();
	//}
	//++count;
	//count = count & 1;
}