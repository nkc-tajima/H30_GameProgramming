// ---------------------------------------------------------------------------------
// GT3A&GK1A ゲームプログラミング
// 三角形を描画
// ---------------------------------------------------------------------------------
#include <tchar.h>			// テキストや文字列を扱うために必要なヘッダーファイル
#include <Windows.h>		// Windowsプログラムで必要になるものが入っている
#include <D3D11.h>			// DirectX11SDKを使うために必要なもの
#include <d3dcompiler.h>
#include <directxcolors.h>	// DirectX::Colorsを利用するために必要

#pragma comment(lib, "d3d11.lib")		// DirectX11ライブラリを利用するために必要
#pragma comment(lib, "d3dCompiler.lib")	// シェーダーコードをコンパイルするために必要

using namespace std;
using namespace DirectX;

// 頂点データ構造体
struct Vertex {
	float pos[3];	// 座標
	float col[4];	// 色
};

// 入力アセンブラ、入力レイアウト
D3D11_INPUT_ELEMENT_DESC g_VertexDesc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

// 画面サイズ設定
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// ウィンドウハンドル ウィンドウを操作するために必要なもの

// DirectXのデバイスのターゲット
D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
UINT					g_FeatureLevels = 3;	  // ターゲットの要素数
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported; // デバイス作成時に返されるバージョン

// インターフェイス
ID3D11Device*			g_pd3dDevice = nullptr;			// レンダリングおよびリソースの作成に使用
ID3D11DeviceContext*	g_pImmediateContext = nullptr;	// レンダリングコマンドを生成するデバイスコンテキスト
IDXGISwapChain*			g_pSwapChain = nullptr;			// フロントバッファとバックバッファを使って画面を更新する
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;	// レンダリング時にレンダーターゲットのサブリソースを識別
D3D11_VIEWPORT			g_ViewPort;						// 描画範囲
ID3D11Texture2D*		g_pDepthStencil;				// 深度/ステンシルリソースとして使用する
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;	// 深度/ステンシルリソースにアクセスする

ID3D11VertexShader*     g_pVertexShader = nullptr;		// 頂点シェーダ
ID3D11PixelShader*      g_pPixelShader = nullptr;		// ピクセルシェーダ

ID3D11InputLayout*      g_pVertexLayout = nullptr;		// 入力アセンブラステージの入力データにアクセス
ID3D11Buffer*           g_pVertexBuffer = nullptr;		// 頂点データを保存する

ID3D11BlendState*		g_pBlendState = nullptr;		// アルファブレンディング 

// 関数のプロトタイプ宣言
void InitWindow(HINSTANCE hInstance);	 // ウィンドウ生成
HRESULT InitDevice();		// デバイスとスワップチェーン初期化
void CleanupDevice();		// デバイスとスワップチェーン解放
void Render();				// 描画

// ウィンドウプロシージャ（コールバック関数）発生したイベントごとに処理を行う
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:		// 閉じるボタンが押されたとき
		PostMessage(hWnd, WM_DESTROY, 0, 0);	// WM_DESTROYへ
		break;
	case WM_DESTROY:	// ウィンドウが終了するとき
		PostQuitMessage(0);	// WM_QUITを送信する
		break;
	}
	// 既定のウィンドウプロシージャを呼び出して処理を返す
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// シェーダーファイルを読み込む
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	// コンパイラがHLSLコードをコンパイルする方法を指定
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS; // 厳密なコンパイルを強制
// デバッグモードのときのみ
#ifdef _DEBUG
	// デバッグ情報をコードの挿入する
	dwShaderFlags |= D3DCOMPILE_DEBUG;			 
	// コード生成中に最適化ステップをスキップする
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;	 
#endif
	// コンパイル後のバイナリデータを保存する変数
	ID3DBlob* pErrorBlob = nullptr;	
	// HLSLコードをバイトコードにコンパイルする
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	// エラーチェック
	if (FAILED(hr))
	{
		// データが保存されている？
		if (pErrorBlob)
		{
			// 文字列をデバッガに送る
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			// 解放
			pErrorBlob->Release();
		}
		return hr;
	}
	// データが入っていたら解放する
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

// ウィンドウ生成
void InitWindow(HINSTANCE hInstance)
{
	// ウィンドウクラス登録
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL003";	// ウィンドウクラスを識別する文字列
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;	// ウィンドウスタイル
	wcex.lpfnWndProc = WndProc;				// ウィンドウのメッセージを処理するコールバック関数へのポインタ
	wcex.cbClsExtra = 0;					// ウィンドウクラス構造体の後ろに割り当てる補足のバイト数
	wcex.cbWndExtra = 0;					// ウィンドウインスタンスの後ろに割り当てる補足のバイト数
	wcex.hInstance = hInstance;				// このクラスのためのウィンドウプロシージャがあるハンドル
	wcex.hIcon = nullptr;					// アイコン
	wcex.hCursor = nullptr;					// マウスカーソル
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// ウィンドウ背景色
	wcex.lpszMenuName = nullptr;			// デフォルトメニュー名
	wcex.lpszClassName = szWindowClass;		// ウィンドウクラス名
	wcex.hIconSm = nullptr;					// 小さいアイコン
	RegisterClassEx(&wcex);					// ウィンドウクラス登録

	// ウィンドウ生成 成功時はウィンドウハンドル、失敗時はnullが返る
	g_hWnd = CreateWindow(
		szWindowClass,				// RegisterClass()で登録したクラスの名前
		"DirectX11 Tutorial003",	// タイトルバー
		WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,// ウィンドウスタイル
		CW_USEDEFAULT,				// ウィンドウ左上x座標
		CW_USEDEFAULT,				// ウィンドウ左上y座標
		WINDOW_W,					// ウィンドウ幅
		WINDOW_H,					// ウィンドウ高さ
		nullptr,					// 親ウィンドウのハンドル
		nullptr,					// メニューのハンドル
		wcex.hInstance,				// ウィンドウを作成するモジュールのハンドル
		nullptr						// WM_CREATEでLPARAMに渡したい値
	);
}

// デバイスとスワップチェーンの初期化
HRESULT InitDevice()
{
	// 初期化が正常に行われたか判断する
	HRESULT hr = S_OK;

	// スワップチェーンの生成
	DXGI_SWAP_CHAIN_DESC desc;					  
	ZeroMemory(&desc, sizeof(desc));	// 初期化
	desc.BufferCount = 1;				// バックバッファの数
	desc.BufferDesc.Width = WINDOW_W;	// バックバッファの幅
	desc.BufferDesc.Height = WINDOW_H;	// バックバッファの高さ
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// 画面に出力するフォーマット
	desc.BufferDesc.RefreshRate.Numerator = 60;			// リフレッシュレート（分子）
	desc.BufferDesc.RefreshRate.Denominator = 1;		// リフレッシュレート（分母）
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;// 走査線の方向を指定
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;// スケーリングのオンオフ設定
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// リソース作成オプション
	desc.OutputWindow = g_hWnd;		// 出力ウィンドウへの関連付け
	desc.SampleDesc.Count = 1;		// ピクセル単位のマルチサンプリングの数
	desc.SampleDesc.Quality = 0;	// マルチサンプリングのクオリティ
	desc.Windowed = TRUE;			// TRUE:ウィンドウモード FALES:フルスクリーンモード
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 画面表示の動作のオプション

	// デバイスとスワップチェインの作成
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,					// デバイスの作成時に使用するビデオアダプタへのポインタ
		D3D_DRIVER_TYPE_HARDWARE,	// 作成するデバイスの種類を表す D3D_DRIVER_TYPE 
		nullptr,					// ソフトウェアラスタライザーを実装するDLLのハンドル
		0,							// 有効にするランタイムレイヤー
		g_pFeatureLevels,			// 作成を試みる機能レベルの順序を指定
		1,							// pFeatureLevelsの要素数
		D3D11_SDK_VERSION,			// SDKのバージョン
		&desc,						// スワップ チェーンの初期化パラメーターを格納するスワップチェインの記述へのポインタ
		&g_pSwapChain,				// レンダリングに使用するスワップチェインを表すIDXGISwapChainオブジェクトへのポインタのアドレスを返す
		&g_pd3dDevice,				// 作成されたデバイスを表すID3D11Deviceオブジェクトへのポインタのアドレスを返す
		&g_FeatureLevelsSupported,	// このデバイスでサポートされている機能レベルの配列にある最初の要素を表すD3D_FEATURE_LEVELへのポインタを返す
		&g_pImmediateContext);		// デバイス コンテキストを表すID3D11DeviceContextオブジェクトへのポインタのアドレスを返す

	if (FAILED(hr))	return E_FAIL;	// エラーチェック


	// バックバッファを描画ターゲットに設定
	// スワップチェインから最初のバックバッファを取得する
	ID3D11Texture2D* pBackBuffer;	// バッファのアクセスに使うインターフェイス(バッファを2Dテクスチャとして扱う)
	hr = g_pSwapChain->GetBuffer(
		0,							// バッファのインデックス
		__uuidof(ID3D11Texture2D),	// バッファにアクセスするインターフェイス
		(LPVOID*)&pBackBuffer		// バックバッファ インターフェイスへのポインタ
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// バックバッファのレンダーターゲットビューを作成
	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,			// レンダーターゲットのオブジェクトへのポインタ
		nullptr,				// レンダーターゲットビューのオブジェクトへのポインタ
		&g_pRenderTargetView	// レンダーターゲットビューのリソースを受け取る変数
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// レンダーターゲットを出力マネージャーのレンダーターゲットとして設定
	g_pImmediateContext->OMSetRenderTargets(
		1,						// 設定するレンダーターゲットの数
		&g_pRenderTargetView,	// デバイスに設定するレンダーターゲットの配列のポインタ
		nullptr			        // デバイスに設定する深度/ステンシルビューのポインタ
	);

	// ビューポートの設定(描画領域)-1.0～1.0の範囲で作られたワールド座標をスクリーン座標に変換するための情報
	g_ViewPort.TopLeftX = 0.0f;		// ビューポート領域の左上X座標
	g_ViewPort.TopLeftY = 0.0f;		// ビューポート領域の左上Y座標
	g_ViewPort.Width = WINDOW_W;	// ビューポート領域の幅
	g_ViewPort.Height = WINDOW_H;	// ビューポート領域の高さ
	g_ViewPort.MinDepth = 0.0f;		// ビューポート領域の深度値の最小値
	g_ViewPort.MaxDepth = 1.0f;		// ビューポート領域の深度値の最大値
	// ラスタライザにビューポートを設定
	g_pImmediateContext->RSSetViewports(
		1,				// 設定するビューポートの数
		&g_ViewPort		// 設定するD3D11_VIEWPORT構造体の配列
	);


	// 深度/ステンシルテクスチャの作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = WINDOW_W;					// 幅
	descDepth.Height = WINDOW_H;				// 高さ
	descDepth.MipLevels = 1;					// ミップマップ レベル数
	descDepth.ArraySize = 1;					// 配列サイズ
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;	// フォーマット(深度のみ)
	descDepth.SampleDesc.Count = 1;				// マルチサンプリングの設定
	descDepth.SampleDesc.Quality = 0;			// マルチサンプリングの品質
	descDepth.Usage = D3D11_USAGE_DEFAULT;		// 使用方法 デフォルト
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// 深度/ステンシルとして使用
	descDepth.CPUAccessFlags = 0;				// 許可するCPUアクセスのフラグ
	descDepth.MiscFlags = 0;					// その他のオプション設定

	// 2Dテクスチャーの配列を作成
	hr = g_pd3dDevice->CreateTexture2D(
		&descDepth,			// 2Dテクスチャの記述へのポインタ
		nullptr,			// サブリソースの記述の配列へのポインタ
		&g_pDepthStencil	// 作成されるテクスチャへのポインタのアドレス
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// 深度/ステンシルビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;	// リソース データのフォーマット
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;// 深度/ステンシルリソースへのアクセス方法を指定
	descDSV.Flags = 0;					// テクスチャーが読み取り専用かどうか
	descDSV.Texture2D.MipSlice = 0;		// 最初に使用するミップマップレベルのインデックス
	
	// 深度/ステンシルビューの作成
	hr = g_pd3dDevice->CreateDepthStencilView(
		g_pDepthStencil,	// 深度/ステンシルサーフェスとして機能するリソースへのポインタ
		&descDSV,			// 深度/ステンシルビューの記述へのポインタ
		&g_pDepthStencilView// 作成したビューを受け取る
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// ブレンディングステートを記述
	D3D11_BLEND_DESC descBlend;
	// 初期化
	ZeroMemory(&descBlend, sizeof(descBlend));
	// ピクセルをレンダーターゲットに設定するとき、
	// アルファトゥカバレッジをマルチサンプリングテクニックとして使用するかどうか
	descBlend.AlphaToCoverageEnable = FALSE;	
	// 同時処理のレンダーターゲットで独立したブレンディングを
	// 有効にするかどうか
	descBlend.IndependentBlendEnable = FALSE;	
	// ブレンディングを有効にする
	descBlend.RenderTarget[0].BlendEnable = TRUE;	
	// 最初のRGBデータソースを指定
	descBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;	
	// 2番目のRGBデータソースを指定
	descBlend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	// RGBデータソースの組み合わせ方法を定義
	descBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;		
	// 最初のアルファデータソースを指定
	descBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	// 2番目のアルファデータソースを指定
	descBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	// アルファデータソースの組み合わせ方法を定義
	descBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	// 書き込みマスク
	descBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  
	// ブレンディングステートを作成
	g_pd3dDevice->CreateBlendState(
		// ブレンディングステートの記述へのポインタ
		&descBlend,			
		// 作成されるオブジェクトへのポインタのアドレス
		&g_pBlendState	 
	);
	// ブレンディング係数の配列
	float blendFactor[4] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO
	};
	// ブレンディングステートの設定
	g_pImmediateContext->OMSetBlendState(
		// ブレンディングステートへのポインタ
		g_pBlendState, 
		// ブレンディング係数の配列、RGBA成分毎に１つある
		blendFactor,
		// 32ビットのサンプルカバレッジ
		0xffffffff
	);

	// 頂点シェーダのデータを保存
	ID3DBlob* pVSBlob = nullptr;
	// ファイルのパスを設定
	wchar_t vs[] = L"VertexShader.hlsl";
	// シェーダーファイルを読み込む
	hr = CompileShaderFromFile(
		vs,			// ファイルパス
		"main",		// エントリーポイントの名前
		"vs_5_0",	// シェーダーのバージョン
		&pVSBlob	// 保存する変数 
	);

	// 頂点シェーダの作成
	hr = g_pd3dDevice->CreateVertexShader(
		// コンパイル済みシェーダへのポインタ
		pVSBlob->GetBufferPointer(),
		// コンパイル済みシェーダのサイズ
		pVSBlob->GetBufferSize(),
		// クラスリンクインターフェイスへのポインタ
		nullptr,
		// シェーダオブジェクトへのポインタのアドレス
		&g_pVertexShader
	);
	
	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// 入力レイアウトの作成
	hr = g_pd3dDevice->CreateInputLayout(
		// 入力アセンブラステージの入力データ型の配列
		g_VertexDesc,
		// 入力要素の配列内の入力データ型の数
		ARRAYSIZE(g_VertexDesc),
		// コンパイル済みシェーダのポインタ
		pVSBlob->GetBufferPointer(),
		// コンパイル済みシェーダのサイズ
		pVSBlob->GetBufferSize(),
		// 作成される入力レイアウトオブジェクトへのポインタ
		&g_pVertexLayout
	);
	// 解放
	pVSBlob->Release();
	
	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// ピクセルシェーダのデータを保存
	ID3DBlob* pPSBlob = nullptr;
	// ファイルのパスを設定
	wchar_t ps[] = L"PixelShader.hlsl";
	// シェーダーファイルを読み込む
	hr = CompileShaderFromFile(
		ps,			// ファイルパス
		"main",		// エントリーポイントの名前
		"ps_5_0", 	// シェーダーのバージョン
		&pPSBlob	// 保存する変数 
	);

	// ピクセルシェーダの作成
	hr = g_pd3dDevice->CreatePixelShader(
		// コンパイル済みシェーダへのポインタ
		pPSBlob->GetBufferPointer(),
		// コンパイル済みシェーダのサイズ
		pPSBlob->GetBufferSize(),
		// クラスリンクインターフェイスへのポインタ
		nullptr,
		// シェーダオブジェクトへのポインタのアドレス
		&g_pPixelShader
	);
	// 解放
	pPSBlob->Release();

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	// 頂点情報の設定
	Vertex g_VertexList[]
	{	 	
		//   x座標	y座標 z座標		 R	   G	 B	   A
		{ { -0.5f,  0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f } },// 1つ目の頂点
		{ {  0.5f, -0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f, 1.0f } },// 2つ目の頂点
		{ { -0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f } },// 3つ目の頂点
	};
	// バッファリソースについて記述
	D3D11_BUFFER_DESC descBuffer;
	// 初期化
	ZeroMemory(&descBuffer, sizeof(descBuffer));
	// バッファで想定されている読み込み及び書き込みの方法を識別
	descBuffer.Usage = D3D11_USAGE_DEFAULT;
	// バッファのサイズ 今回は三角形を表示するのでサイズは3つ
	descBuffer.ByteWidth = sizeof(Vertex) * 3;
	// バッファをどのようにグラフィックスパイプラインにバインドするかを識別
	descBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUのアクセスフラグ
	descBuffer.CPUAccessFlags = 0;

	// サブリソースの初期化に使用されるデータを指定
	D3D11_SUBRESOURCE_DATA InitData;
	// 初期化
	ZeroMemory(&InitData, sizeof(InitData));
	// 初期化データへのポインタ
	InitData.pSysMem = g_VertexList;
	// テクスチャにある1本の線の先端から隣の線までの距離
	InitData.SysMemPitch = 0;
	// 3Dテクスチャに関連する値
	InitData.SysMemSlicePitch = 0;

	// バッファの作成
	hr = g_pd3dDevice->CreateBuffer(
		// バッファの記述へのポインタ
		&descBuffer,
		// 初期化データへのポインタ
		&InitData, 
		// 作成されるバッファへのポインタのアドレス
		&g_pVertexBuffer
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

	return S_OK;
}

// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// ウィンドウ生成
	ShowWindow(g_hWnd, nCmdShow);	// ウィンドウ表示
	UpdateWindow(g_hWnd);			// ウィンドウ更新

	// デバイスとスワップチェーンの初期化
	if (FAILED(InitDevice()))
	{
		// 失敗したら終了
		return 0;
	}

	MSG hMsg = {};
	ZeroMemory(&hMsg, sizeof(hMsg));// 初期化
	while (hMsg.message != WM_QUIT)	// 終了が押されていない間ループする
	{
		if (PeekMessage(&hMsg, nullptr, 0, 0, PM_REMOVE))// メッセージを取得する。出来なくても繰り返し処理を行う
		{
			TranslateMessage(&hMsg);// キーボードメッセージから文字メッセージを生成する
			DispatchMessage(&hMsg);	// ウィンドウプロシージャにメッセージを渡す
		}
		else
		{
			// 描画処理
			Render();
		}
	}

	// デバイスとスワップチェーンの解放
	CleanupDevice();

	return 0;
}

// 描画処理
void Render()
{
	// 背景の色を設定 レンダーターゲットクリア
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::Black);
	// 深度/ステンシルバッファクリア
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 入力アセンブラステージに入力レイアウトオブジェクトをバインド
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	// 入力アセンブラステージに頂点バッファの配列をバインド
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pImmediateContext->IASetVertexBuffers(
		0,					// バインドに使用する最初の入力スロット
		1,					// 配列内の頂点バッファの数
		&g_pVertexBuffer,	// 頂点バッファの配列へのポインタ
		&strides,			// ストライド値
		&offsets			// オフセット値
	);
	// 頂点シェーダをデバイスに設定
	g_pImmediateContext->VSSetShader(
		g_pVertexShader,	// 頂点シェーダへのポインタ
		nullptr,			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	// ピクセルシェーダをデバイスに設定
	g_pImmediateContext->PSSetShader(
		g_pPixelShader,		// ピクセルシェーダへのポインタ
		nullptr, 			// クラスインスタンスインターフェイスの配列へのポインタ
		0					// 配列のクラスインスタンスインターフェイスの数
	);
	// 入力アセンブラステージにプリミティブの形状をバインド
	g_pImmediateContext->IASetPrimitiveTopology(
		// 頂点データを三角形のリストとして解釈
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		// 複数の三角形を出したいときはTRIANGLESTRIPにすると頂点を節約できる
	);
	// プリミティブを描画
	g_pImmediateContext->Draw(
		3,		// 描画する頂点の数
		0		// 最初の頂点のインデックス
	);
	// レンダリングされたイメージを画面に描画する
	g_pSwapChain->Present(0, 0);

}

// デバイスとスワップチェーンの解放
void CleanupDevice()
{
	// ステートの解放
	if (g_pImmediateContext)	g_pImmediateContext->ClearState();

	// インターフェイスの解放
	if (g_pDepthStencilView)	g_pDepthStencilView->Release();
	if (g_pDepthStencil)		g_pDepthStencil->Release();
	if (g_pRenderTargetView)	g_pRenderTargetView->Release();
	if (g_pSwapChain)			g_pSwapChain->Release();
	if (g_pImmediateContext)	g_pImmediateContext->Release();
	if (g_pd3dDevice)			g_pd3dDevice->Release();
	if (g_pVertexShader)		g_pVertexShader->Release();
	if (g_pPixelShader)			g_pPixelShader->Release();
	if (g_pVertexLayout)		g_pVertexLayout->Release();
	if (g_pVertexBuffer)		g_pVertexBuffer->Release();
	if (g_pBlendState)			g_pBlendState->Release();
}

