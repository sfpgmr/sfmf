#include "pch.h"
#include "SampleFpsTextRenderer.h"

#include "Common/DirectXHelper.h"

using namespace sfmf;

// テキスト レンダリングで使用する D2D リソースを初期化します。
SampleFpsTextRenderer::SampleFpsTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) : 
	m_text(L""),
	m_deviceResources(deviceResources)
{
	ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

	// デバイスに依存するリソースを作成します。
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			48.0f,
			L"en-US",
			&m_textFormat
			)
		);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);

	CreateDeviceDependentResources();
}

// 表示するテキストを更新します。
void SampleFpsTextRenderer::Update(DX::StepTimer const& timer)
{
	// 表示するテキストを更新します。
	uint32 fps = timer.GetFramesPerSecond();

	m_text = (fps > 0) ? std::to_wstring(fps) + L" FPS" : L" - FPS";

	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			m_text.c_str(),
			(uint32) m_text.length(),
			m_textFormat.Get(),
			240.0f, // 入力テキストの最大幅。
			50.0f, // 入力テキストの最大高さ。
			&m_textLayout
			)
		);

	DX::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
		);
}

// フレームを画面に描画します。
void SampleFpsTextRenderer::Render(ID2D1Bitmap1* targetBitmap)
{
	if (!targetBitmap) return;

	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	context->SetTarget(targetBitmap);
	Windows::Foundation::Size logicalSize = { (float)targetBitmap->GetPixelSize().width, (float)targetBitmap->GetPixelSize().height};//m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// 右下隅に配置
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		logicalSize.Width - m_textMetrics.layoutWidth,
		logicalSize.Height - m_textMetrics.height
		);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
		);

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		m_textLayout.Get(),
		m_whiteBrush.Get()
		);

	// D2DERR_RECREATE_TARGET をここで無視します。このエラーは、デバイスが失われたことを示します。
	// これは、Present に対する次回の呼び出し中に処理されます。
	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void SampleFpsTextRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
		);
}
void SampleFpsTextRenderer::ReleaseDeviceDependentResources()
{
	m_whiteBrush.Reset();
}