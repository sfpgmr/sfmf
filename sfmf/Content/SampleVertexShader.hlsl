// ジオメトリを作成するために 3 つの基本的な列優先のマトリックスを保存する定数バッファー。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// 頂点シェーダーへの入力として使用する頂点ごとのデータ。
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

// ピクセル シェーダーを通じて渡されるピクセルごとの色データ。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

// GPU で頂点処理を行うための簡単なシェーダー。
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// 頂点の位置を、射影された領域に変換します。
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// 変更せずに色をパススルーします。
	output.color = input.color;

	return output;
}
