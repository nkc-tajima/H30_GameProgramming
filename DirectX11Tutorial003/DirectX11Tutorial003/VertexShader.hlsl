// ---------------------------------------------------------------------------------
// GT3A&GK1A ゲームプログラミング
// 三角形を描画：頂点シェーダ
// ---------------------------------------------------------------------------------
// CPUから受け取る頂点バッファ
struct VS_IN
{
	// D3D11_INPUT_ELEMENT_DESCで定義したセマンティクス名・インデックス番号
	float4 pos : POSITION0;
	float4 col : COLOR0;
};
// ピクセルシェーダに渡す値
struct VS_OUT
{
	// 既に頂点シェーダで処理がされているので
	// システム上で扱う座標情報としてSV_POSITIONを渡す
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};
// エントリーポイント
VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.pos = input.pos;
	output.col = input.col;
	return output;
}