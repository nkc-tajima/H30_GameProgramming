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
cbuffer ConstantBuffer : register(b0)
{
	matrix World;		// ワールド座標行列
	matrix View;		// ビュー変換行列
	matrix Projection;	// プロジェクション変換行列
}


VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.pos = mul(input.pos, World);			// 現在の座標とワールド変換行列で乗算を行う
	output.pos = mul(output.pos, View);			// 上記計算後の座標とビュー変換行列で乗算を行う
	output.pos = mul(output.pos, Projection);	// 上記計算後の座標とプロジェクション変換行列で乗算を行う

	output.col = input.col;
	return output;
}