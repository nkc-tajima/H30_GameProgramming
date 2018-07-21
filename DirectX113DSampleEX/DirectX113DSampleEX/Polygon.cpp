#include "Polygon.h"
#include <fstream>
#include <string>
#include <stdlib.h>

D3D11_INPUT_ELEMENT_DESC vertexDesc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


void CPolygon::Init(ID3D11Device* device,std::string vsStr, std::string psStr)
{
	CreateVS(device, vsStr);
	CreatePS(device, psStr);

	for (int i = 0; i < 4; i++)
	{
		// 頂点の位置
		vertexList[i].pos[0] = i < 2 ? -0.5f : 0.5f;
		vertexList[i].pos[1] = i % 2 == 0 ? -0.5f : 0.5f;
		vertexList[i].pos[2] = 0.0f;
		// 頂点の色
		vertexList[i].col[0] = 1.0f;
		vertexList[i].col[1] = 1.0f;
		vertexList[i].col[2] = 1.0f;
		vertexList[i].col[3] = 1.0f;
		// uv
		vertexList[i].uv[0] = i < 2 ? 0.0f : 1.0f;
		vertexList[i].uv[1] = i % 2 == 0 ? 1.0f : 0.0f;
	}

	// バッファリソースについて記述
	D3D11_BUFFER_DESC descBuffer;
	// 初期化
	ZeroMemory(&descBuffer, sizeof(descBuffer));
	// バッファで想定されている読み込み及び書き込みの方法を識別
	descBuffer.Usage = D3D11_USAGE_DEFAULT;
	// バッファのサイズ 今回は三角形を表示するのでサイズは3つ
	descBuffer.ByteWidth = sizeof(vertex) * 4;
	// バッファをどのようにグラフィックスパイプラインにバインドするかを識別
	descBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUのアクセスフラグ
	descBuffer.CPUAccessFlags = 0;

	// サブリソースの初期化に使用されるデータを指定
	D3D11_SUBRESOURCE_DATA InitData;
	// 初期化
	ZeroMemory(&InitData, sizeof(InitData));
	// 初期化データへのポインタ
	InitData.pSysMem = vertexList;
	// テクスチャにある1本の線の先端から隣の線までの距離
	InitData.SysMemPitch = 0;
	// 3Dテクスチャに関連する値
	InitData.SysMemSlicePitch = 0;
	// バッファの作成
	device->CreateBuffer(
		// バッファの記述へのポインタ
		&descBuffer,
		// 初期化データへのポインタ
		&InitData,
		// 作成されるバッファへのポインタのアドレス
		&pVertexBuffer
	);
}

void CPolygon::Render(ID3D11DeviceContext* context, ID3D11Buffer* buffer)
{
	// サブリソースの更新
	context->UpdateSubresource(
		pVertexBuffer,
		0,
		nullptr,
		vertexList,
		0,
		0
	);
	// 入力アセンブラステージに入力レイアウトオブジェクトをバインド
	context->IASetInputLayout(pVertexLayout);
	// 入力アセンブラステージに頂点バッファの配列をバインド
	UINT strides = sizeof(vertex);
	UINT offsets = 0;
	context->IASetVertexBuffers(
		0,					// バインドに使用する最初の入力スロット
		1,					// 配列内の頂点バッファの数
		&pVertexBuffer,	// 頂点バッファの配列へのポインタ
		&strides,			// ストライド値
		&offsets			// オフセット値
	);
	// 頂点シェーダをデバイスに設定
	context->VSSetShader(
		pVertexShader,// 頂点シェーダへのポインタ
		nullptr,			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	// 頂点シェーダのパイプラインステージで使用される定数バッファを設定
	context->VSSetConstantBuffers(
		0,					// デバイスの配列の中で定数バッファの設定を開始する位置
		1,					// 設定するバッファの数
		&buffer	// デバイスに設定する定数バッファの配列
	);

	// ピクセルシェーダをデバイスに設定
	context->PSSetShader(
		pPixelShader,		// ピクセルシェーダへのポインタ
		nullptr, 			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	// 入力アセンブラステージにプリミティブの形状をバインド
	context->IASetPrimitiveTopology(
		// 頂点データを三角形のリストとして解釈
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
		// 複数の三角形を出したいときはTRIANGLESTRIPにすると頂点を節約できる
	);
	// プリミティブを描画
	context->Draw(
		4,		// 描画する頂点の数
		0		// 最初の頂点のインデックス
	);

}
void CPolygon::Render(ID3D11DeviceContext* context, ID3D11Buffer* buffer, ID3D11ShaderResourceView* texture, ID3D11SamplerState* sampler)
{
	// サブリソースの更新
	context->UpdateSubresource(
		pVertexBuffer,
		0,
		nullptr,
		vertexList,
		0,
		0
	);
	// 入力アセンブラステージに入力レイアウトオブジェクトをバインド
	context->IASetInputLayout(pVertexLayout);
	// 入力アセンブラステージに頂点バッファの配列をバインド
	UINT strides = sizeof(vertex);
	UINT offsets = 0;
	context->IASetVertexBuffers(
		0,					// バインドに使用する最初の入力スロット
		1,					// 配列内の頂点バッファの数
		&pVertexBuffer,	// 頂点バッファの配列へのポインタ
		&strides,			// ストライド値
		&offsets			// オフセット値
	);
	// 頂点シェーダをデバイスに設定
	context->VSSetShader(
		pVertexShader,// 頂点シェーダへのポインタ
		nullptr,			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	// 頂点シェーダのパイプラインステージで使用される定数バッファを設定
	context->VSSetConstantBuffers(
		0,					// デバイスの配列の中で定数バッファの設定を開始する位置
		1,					// 設定するバッファの数
		&buffer	// デバイスに設定する定数バッファの配列
	);

	// ピクセルシェーダをデバイスに設定
	context->PSSetShader(
		pPixelShader,		// ピクセルシェーダへのポインタ
		nullptr, 			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	//テクスチャーをシェーダーに渡す
	context->PSSetSamplers(
		0,
		1,
		&sampler
	);
	context->PSSetShaderResources(
		0, 
		1, 
		&texture
	);	// 入力アセンブラステージにプリミティブの形状をバインド
	context->IASetPrimitiveTopology(
		// 頂点データを三角形のリストとして解釈
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
		// 複数の三角形を出したいときはTRIANGLESTRIPにすると頂点を節約できる
	);
	// プリミティブを描画
	context->Draw(
		4,		// 描画する頂点の数
		0		// 最初の頂点のインデックス
	);

}

void CPolygon::Release()
{
	if (pVertexLayout)pVertexLayout->Release();
	if (pVertexBuffer)pVertexBuffer->Release();
	if (pVertexShader)pVertexShader->Release();
	if (pPixelShader) pPixelShader->Release();
}
void CPolygon::SetColor(float r, float g, float b, float a, int v)
{
	vertexList[v].col[0] = r;
	vertexList[v].col[1] = g;
	vertexList[v].col[2] = b;
	vertexList[v].col[3] = a;
}

// シェーダーファイル読み込み
HRESULT CPolygon::CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	// コンパイラがHLSLコードをコンパイルする方法を指定
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;// 厳密なコンパイルを強制
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;				// デバッグファイル/行/型/シンボル情報を出力コードに挿入するようにコンパイラに指示

	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;	// コード生成中に最適化ステップをスキップするようにコンパイラに指示
#endif

	ID3DBlob* pErrorBlob = nullptr;					// コンパイル後のバイナリデータを取得する
													// ファイルからシェーダをコンパイル
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	// エラーチェック
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			// 文字列をデバッガに送る
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT CPolygon::CreateVS(ID3D11Device* device, std::string vsStr)
{
	// 生成が正常に行われたか判断する為に用意
	HRESULT hr = S_OK;
	// ファイルの拡張子を取得する
	int num = vsStr.find_last_of('.');
	std::string extension = vsStr.substr(num + 1);
	// stringからconst wchar_tに変換する
	wchar_t *vs = new wchar_t[vsStr.length() + 1];
	size_t value = 0;
	mbstowcs_s(&value, vs, 20, vsStr.c_str(), vsStr.length() + 1);


	// 読み込む拡張子がhlsl?
	if (extension == "hlsl")
	{
		ID3DBlob* pVSBlob = nullptr;			// オブジェクトコードとエラーメッセージを返す
												// シェーダーファイル読み込み
		hr = CompileShaderFromFile(
			vs,					// ファイルパス
			"main",				// エントリーポイントの名前
			"vs_5_0",			// シェーダーのバージョン
			&pVSBlob			// 保存する変数 
		);
		if (FAILED(hr))	return E_FAIL;	// エラーチェック

		// 頂点シェーダの作成
		hr = device->CreateVertexShader(
			pVSBlob->GetBufferPointer(),	// コンパイル済みシェーダへのポインタ
			pVSBlob->GetBufferSize(),		// コンパイル済みシェーダのサイズ
			nullptr,						// クラスリンクインターフェイスへのポインタ
			&pVertexShader					// ID3D11VertexShaderインターフェイスへのポインタのアドレス
		);
		if (FAILED(hr))	return E_FAIL;	// エラーチェック

		// インプットレイアウトの作成
		hr = device->CreateInputLayout(
			vertexDesc,					// 入力データ型の配列
			ARRAYSIZE(vertexDesc),		// 配列内の入力データ型の数
			pVSBlob->GetBufferPointer(),// コンパイル済みシェーダへのポインタ
			pVSBlob->GetBufferSize(),	// コンパイル済みシェーダのサイズ
			&pVertexLayout				// 作成される入力レイアウトオブジェクトへのポインタ
		);
		pVSBlob->Release();				// 解放		
		if (FAILED(hr))	return E_FAIL;	// エラーチェック
	}
	// 読み込む拡張子がcso?
	else if (extension == "cso")
	{
		ReadCSOFile(vs);

		// 頂点シェーダの作成
		hr = device->CreateVertexShader(
			pCsoBin,						// コンパイル済みシェーダへのポインタ
			CsoSize,						// コンパイル済みシェーダのサイズ
			nullptr,						// クラスリンクインターフェイスへのポインタ
			&pVertexShader					// ID3D11VertexShaderインターフェイスへのポインタのアドレス
		);
		if (FAILED(hr))	return E_FAIL;	// エラーチェック

										// インプットレイアウトの作成
		hr = device->CreateInputLayout(
			vertexDesc,					// 入力データ型の配列
			ARRAYSIZE(vertexDesc),		// 配列内の入力データ型の数
			pCsoBin,					// コンパイル済みシェーダへのポインタ
			CsoSize,					// コンパイル済みシェーダのサイズ
			&pVertexLayout				// 作成される入力レイアウトオブジェクトへのポインタ
		);
	}

	return hr;
}


HRESULT CPolygon::CreatePS(ID3D11Device* device, std::string psStr)
{
	// 生成が正常に行われたか判断する為に用意
	HRESULT hr = S_OK;	
	// ファイルの拡張子を取得する
	int num = psStr.find_last_of('.');
	std::string extension = psStr.substr(num + 1);
	// stringからconst wchar_tに変換する
	wchar_t *ps = new wchar_t[psStr.length() + 1];
	size_t value = 0;
	mbstowcs_s(&value, ps, 20, psStr.c_str(), psStr.length() + 1);


	// 読み込む拡張子がhlsl?
	if (extension == "hlsl")
	{
		ID3DBlob* pPSBlob = nullptr;			// オブジェクトコードとエラーメッセージを返す
	// シェーダーファイル読み込み
		hr = CompileShaderFromFile(
			ps,					// ファイルパス
			"main",				// エントリーポイントの名前
			"ps_5_0",			// シェーダーのバージョン
			&pPSBlob			// 保存する変数 
		);
		if (FAILED(hr))	return E_FAIL;	// エラーチェック

		// ピクセルシェーダの作成
		hr = device->CreatePixelShader(
			pPSBlob->GetBufferPointer(),// コンパイル済みシェーダへのポインタ
			pPSBlob->GetBufferSize(),	// コンパイル済みシェーダのサイズ
			nullptr,					// クラスリンクインターフェイスへのポインタ
			&pPixelShader				// ID3D11PixelShaderインターフェイスへのポインタのアドレス
		);
		pPSBlob->Release();				// 解放		

		if (FAILED(hr))	return E_FAIL;	// エラーチェック
	}
	// 読み込む拡張子がcso?
	else if (extension == "cso")
	{
		ReadCSOFile(ps);

		// ピクセルシェーダの作成
		hr = device->CreatePixelShader(
			pCsoBin,					// コンパイル済みシェーダへのポインタ
			CsoSize,					// コンパイル済みシェーダのサイズ
			nullptr,					// クラスリンクインターフェイスへのポインタ
			&pPixelShader				// ID3D11PixelShaderインターフェイスへのポインタのアドレス
		);

	}
	return hr;
}

// CSOファイルを読み込む
void  CPolygon::ReadCSOFile(LPCWSTR szFileName)
{
	std::ifstream binfile(szFileName, std::ios::in | std::ios::binary);

	if (binfile.is_open()) {
		int fsize = static_cast<int>(binfile.seekg(0, std::ios::end).tellg());
		binfile.seekg(0, std::ios::beg);
		char* code(new char[fsize]);
		binfile.read(code, fsize);
		CsoSize = fsize;
		pCsoBin = std::move(code);
	}	
}
