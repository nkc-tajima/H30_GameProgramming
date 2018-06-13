// 受け取る頂点データの値
struct VS_IN
{
	float4 pos : POSITION0;// D3D11_INPUT_ELEMENT_DESCで定義したセマンティクス名・インデックス番号の要素を受け取る
	float4 col : COLOR0;
};

// ピクセルシェーダーに送る値
struct VS_OUT
{
	// 既に頂点シェーダで処理がされてあるので、システム上で扱う座標情報としてSV_POSITIONになる
	// SVはSystem Valueの略？
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};
// 座標行列データを受け取る
cbuffer ConstantBuffer:register(b0)
{
	matrix World;		// ワールド座標行列
	matrix View;		// ビュー座標行列
	matrix Projection;	// プロジェクション座標行列
}

VS_OUT main(VS_IN input)
{
	VS_OUT output;

	//output.pos = input.pos;
	// 現在の座標とワールド座標行列で行列の乗算を行う
	output.pos = mul(input.pos, World);
	// 上記の計算後の値とビュー座標行列で行列の乗算を行う
	output.pos = mul(output.pos, View);
	// 上記の計算後の値とプロジェクション座標行列で行列の乗算を行う
	output.pos = mul(output.pos, Projection);

	output.col = input.col;
	return output;
}