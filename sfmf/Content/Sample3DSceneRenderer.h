#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "sfhelper.h"
#include "sfmf.h"

namespace sfmf
{
	// このサンプル レンダリングでは、基本的なレンダリング パイプラインをインスタンス化します。
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void RenderToTexture();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }

		void WriteVideoFrame();
		ID2D1Bitmap1* GetvideoTextureBitmap(){ return m_videoBitmap.Get(); }


	private:
		void Rotate(float radians);

	private:

		// デバイス リソースへのキャッシュされたポインター

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// キューブ ジオメトリの Direct3D リソース

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;


		// 動画テクスチャ用リソース

		//ID3D11Texture2DPtr m_videoSrcTexure;
		ID3D11ShaderResourceViewPtr    m_videoResourceView;
		ID3D11RenderTargetViewPtr m_videoRenderTargetView;
		ID3D11DepthStencilViewPtr m_videoDepthView;
		ID3D11Texture2DPtr m_videoTexture;
		ID3D11Texture2DPtr m_videoDepthTexture;
		ID3D11Texture2DPtr m_videoStageTexture;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_videoVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_videoPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_videoVertexLayout;
		ID3D11BufferPtr                 m_videoVertex;
		ID3D11RasterizerState1Ptr  m_videoRasterState;
		ID3D11SamplerStatePtr m_videoSamplerState;
		float m_clearColor[4];
		CD3D11_VIEWPORT m_videoViewport;

		ID2D1Bitmap1Ptr m_videoBitmap;


		// キューブ ジオメトリのシステム リソース。
		
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// レンダリング ループで使用する変数。
		
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

