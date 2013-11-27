#pragma once

namespace sfmf
{
	// MVP マトリックスを頂点シェーダーに送信するために使用する定数バッファー。
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// 頂点シェーダーへの頂点ごとのデータの送信に使用します。
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexVideo {
		DirectX::XMFLOAT2 pos;	//x,y,z
		DirectX::XMFLOAT2 uv;	//u,v
	};

	struct PNTVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 textureCoordinate;
	};
}