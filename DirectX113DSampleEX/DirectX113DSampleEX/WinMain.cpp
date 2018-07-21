#include <tchar.h>			// テキストや文字列を扱うために必要なヘッダーファイル
#include <Windows.h>		// Windowsプログラムで必要になるものが入っている
#include <D3D11.h>			// DirectX11SDKを使うために必要なもの
#include <d3dcompiler.h>	// シェーダーファイルをコンパイルするために必要
#include <directxcolors.h>	// DirectX::Colorsを利用するために必要

#include "Polygon.h"
#include <random>

#pragma comment(lib, "d3d11.lib")		// DirectX11ライブラリを利用するために必要
#pragma comment(lib, "d3dCompiler.lib")	// シェーダーコードをコンパイルするために必要


using namespace std;
using namespace DirectX;


// 座標変換構造体
struct ConstantBuffer
{
	XMMATRIX world;			// ワールド変換行列
	XMMATRIX view;			// ビュー変換行列
	XMMATRIX projection;	// プロジェクション変換行列
};

// 画面サイズ設定
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// ウィンドウハンドル ウィンドウを操作するために必要なもの

						// Direct3Dデバイスのターゲットとなる機能セットを記述
D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
UINT					g_FeatureLevels = 3;	  // 配列の要素数
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported; // デバイス作成時に返される機能レベル

												  // インターフェイス
ID3D11Device*			g_pd3dDevice = nullptr;			// レンダリングおよびリソースの作成に使用
ID3D11DeviceContext*	g_pImmediateContext = nullptr;	// レンダリングコマンドを生成するデバイスコンテキスト
IDXGISwapChain*			g_pSwapChain = nullptr;			// フロントバッファとバックバッファを使って画面を更新する
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;	// 描画するためのリソースを作成し、実際の描画操作を行う
D3D11_VIEWPORT			g_ViewPort;						// 描画範囲
ID3D11Texture2D*		g_pDepthStencil;				// 深度/ステンシルリソースとして使用する
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;	// 深度/ステンシルリソースにアクセスする

ID3D11VertexShader*     g_pVertexShader = nullptr;		// 頂点シェーダ
ID3D11PixelShader*      g_pPixelShader = nullptr;		// ピクセルシェーダ

ID3D11InputLayout*      g_pVertexLayout = nullptr;		// 頂点データの構造を定義
ID3D11Buffer*           g_pVertexBuffer = nullptr;		// 頂点データを保存

ID3D11BlendState*		g_pBlendState = nullptr;		// アルファブレンディング
ID3D11RasterizerState*  g_pRasterizerState = nullptr;	// ラスタライザ

														// 座標変換用変数定義
ID3D11Buffer*           g_pConstantBuffer = nullptr;	// 座標系データを保存
XMMATRIX				g_World;						// ワールド変換行列
XMMATRIX				g_View;							// ビュー変換行列
XMMATRIX				g_Proj;							// プロジェクション変換行列

#define SIZE_X 80
#define SIZE_Y 48
CPolygon				g_Polygon[SIZE_Y][SIZE_X];		// 出したい数だけポリゴン用意
XMMATRIX				g_World2[SIZE_Y][SIZE_X];		// ワールド変換行列
XMFLOAT3				g_pos[SIZE_Y][SIZE_X];			// ポリゴン毎の座標
float					g_fallSpeed[SIZE_Y][SIZE_X];	// ポリゴン毎の落ちる速度
XMFLOAT3				g_rot[SIZE_Y][SIZE_X];			// ポリゴン毎の回転
XMFLOAT3				g_rotSpeed[SIZE_Y][SIZE_X];		// ポリゴン毎の回転速度

// メルセンヌ・ツイスター法による疑似乱数生成器をハードウェア乱数をシードにして初期化
std::random_device seed_gen;
std::mt19937 engine(seed_gen());
std::uniform_real_distribution<> fallDist(-0.5, -0.1);
std::uniform_real_distribution<> rotDist(-0.1, 0.1);

void InitWindow(HINSTANCE hInstance);	 // ウィンドウ生成
HRESULT InitDevice();		// デバイスとスワップチェーン初期化
void CleanupDevice();		// デバイスとスワップチェーン解放
void Render();				// ポリゴン描画


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
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)// escキーでウィンドウ閉じろ
			DestroyWindow(hWnd);
		break;
	}
	// 既定のウィンドウプロシージャを呼び出して処理を返す
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// ウィンドウ生成
void InitWindow(HINSTANCE hInstance)
{
	// ウィンドウクラス登録
	TCHAR szWindowClass[] = "DIRECTX11 SAMPLE EX";	// ウィンドウクラスを識別する文字列
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
		"DirectX11 SampleEX",	// タイトルバー
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
// D3D初期化
HRESULT InitDevice()
{
	HRESULT hr = S_OK;					// 生成が正常に行われたか判断する為に用意

	DXGI_SWAP_CHAIN_DESC desc;			// スワップ チェーンを記述
	ZeroMemory(&desc, sizeof(desc));	// 初期化
	desc.BufferCount = 1;				// バックバッファの数
	desc.BufferDesc.Width = WINDOW_W;	// バックバッファの幅
	desc.BufferDesc.Height = WINDOW_H;	// バックバッファの高さ
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// フォーマット
	desc.BufferDesc.RefreshRate.Numerator = 60;			// リフレッシュレート（分子）
	desc.BufferDesc.RefreshRate.Denominator = 1;		// リフレッシュレート（分母）
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;// 走査線描画
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;// スケーリング
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// バックバッファの使用方法
	desc.OutputWindow = g_hWnd;		// 出力ウィンドウへの関連付け
	desc.SampleDesc.Count = 1;		// ピクセル単位のマルチサンプリングの数
	desc.SampleDesc.Quality = 0;	// マルチサンプリングのクオリティ
	desc.Windowed = TRUE;			// TRUE:ウィンドウモード FALES:フルスクリーンモード
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // スワップ チェーンの動作のオプション

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
		0,							// バックバッファの番号
		__uuidof(ID3D11Texture2D),	// バッファにアクセスするインターフェイス
		(LPVOID*)&pBackBuffer		// バッファを受け取る変数
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

									// バックバッファの描画ターゲットビューを作る
	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,			// ビューでアクセスするリソース
		nullptr,				// 描画ターゲットビューの定義
		&g_pRenderTargetView	// 描画ターゲットビューを受け取る変数
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

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
	descDepth.CPUAccessFlags = 0;				// CPUからアクセスしない
	descDepth.MiscFlags = 0;					// その他の設定なし
												// 2Dテクスチャーの配列を作成
	hr = g_pd3dDevice->CreateTexture2D(
		&descDepth,			// 作成する2Dテクスチャ
		nullptr,			// サブリソースの記述の配列へのポインタ
		&g_pDepthStencil	// 作成したテクスチャを受け取る
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

									// 深度/ステンシルビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;	// リソース データのフォーマット
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;// リソースのタイプ
	descDSV.Flags = 0;					// テクスチャーが読み取り専用かどうか
	descDSV.Texture2D.MipSlice = 0;		// 2Dテクスチャーのサブリソースを指定
	hr = g_pd3dDevice->CreateDepthStencilView(
		g_pDepthStencil,	// 深度/ステンシルビューを作るテクスチャ
		&descDSV,			// 深度/ステンシルビューの設定
		&g_pDepthStencilView// 作成したビューを受け取る
	);

	if (FAILED(hr))	return E_FAIL;	// エラーチェック

									// 描画ターゲットビューを出力マージャーの描画ターゲットとして設定
	g_pImmediateContext->OMSetRenderTargets(
		1,						// 描画ターゲットの数
		&g_pRenderTargetView,	// 描画ターゲットビューの配列
		g_pDepthStencilView     // 深度/ステンシルビューを設定しない
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

	// ラスタライズステートオブジェクトの設定
	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,	// レンダリング時に使用する描画モードを設定
		D3D11_CULL_NONE,	// カリングの設定
		FALSE,				// 三角形が前向きか後ろ向きかを設定する
		0,					// 指定のピクセルに加算する深度値
		0.0f,				// ピクセルの最大深度バイアス
		0.0f,				// ピクセルの傾斜に基づいてバイアスをかける
		FALSE,				// 距離に基づいてクリッピングを有効にする
		FALSE,				// シザー矩形カリングを有効にする
		FALSE,				// マルチサンプリングのアンチエイリアシングを有効にする
		FALSE				// 線のアンチエイリアシングを有効にする
	};
	// ラスタライズステートオブジェクトの作成
	g_pd3dDevice->CreateRasterizerState(
		&descRast,					// ラスタライザステートの記述へのポインタ
		&g_pRasterizerState			// 作成されるラスタライザステートオブジェクトへのポインタのアドレス
	);
	//ラスタライザーをコンテキストに設定
	g_pImmediateContext->RSSetState(g_pRasterizerState);

	// ブレンディングステートの作成
	D3D11_BLEND_DESC descBlend;
	ZeroMemory(&descBlend, sizeof(descBlend));
	descBlend.AlphaToCoverageEnable = FALSE;		// ピクセルをレンダーターゲットに設定するときに、アルファトゥカバレッジをマルチサンプリングテクニックとして使用するかどうかを決定
	descBlend.IndependentBlendEnable = FALSE;		// 同時処理のレンダーターゲットで独立したブレンディングを有効にするかどうか
	descBlend.RenderTarget[0].BlendEnable = TRUE;	// ブレンディングを有効にする
	descBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;		// 最初のRGBデータソースを指定
	descBlend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	// 2番目のRGBデータソースを指定
	descBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;			// RGBデータソースの組合せ方法を定義
	descBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;		// 最初のアルファデータソースを指定
	descBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;	// 2番目のアルファデータソースを指定
	descBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;	// アルファデータソースの組合せ方法を定義
	descBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 書き込みマスク
	g_pd3dDevice->CreateBlendState(
		&descBlend,		// ブレンディングステートの記述へのポインタ
		&g_pBlendState	// 作成されるオブジェクトへのポインタのアドレス
	);
	// ブレンディングステートの設定
	float blendFactor[4] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO
	};
	// ブレンディングステートの設定
	g_pImmediateContext->OMSetBlendState(
		g_pBlendState,	// ブレンディングステートへのポインタ
		blendFactor,	// ブレンディング係数の配列
		0xffffffff		// サンプルマスク
	);

	// 座標変換行列についての記述
	D3D11_BUFFER_DESC descCBuffer;
	// 初期化
	ZeroMemory(&descCBuffer, sizeof(descCBuffer));
	// バッファで想定されている読み込み及び書き込みの方法を識別
	descCBuffer.Usage = D3D11_USAGE_DEFAULT;
	// バッファのサイズ
	descCBuffer.ByteWidth = sizeof(ConstantBuffer);
	// バッファをどのようにグラフィックスパイプラインにバインドするかを識別
	descCBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// CPUのアクセスフラグ
	descCBuffer.CPUAccessFlags = 0;

	// バッファの作成
	hr = g_pd3dDevice->CreateBuffer(
		// バッファの記述へのポインタ
		&descCBuffer,
		// 初期化データへのポインタ
		nullptr,
		// 作成されるバッファへのポインタのアドレス
		&g_pConstantBuffer
	);
	if (FAILED(hr))	return E_FAIL;	// エラーチェック

									// 単位行列の作成
	g_World = XMMatrixIdentity();

	// カメラの位置
	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -50.0f, 0.0f);
	// カメラの焦点
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	// 現在のワールド座標における上方向の定義
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	// カメラの位置、上方向、および焦点を使用して、左手座標系のビュー行列を作成
	g_View = XMMatrixLookAtLH(eye, focus, up);
	// 視野角の設定
	float    fov = XMConvertToRadians(45.0f);
	// アスペクト比の設定
	float    aspect = g_ViewPort.Width / g_ViewPort.Height;
	// ニア（見える範囲の前方）の設定
	float    nearZ = 0.1f;
	// ファー（見える範囲の後方）の設定
	float    farZ = 1000.0f;
	// 視野に基づいて、左手座標系のパースペクティブ射影行列を作成
	g_Proj = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);


	return S_OK;
}
// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// ウィンドウ生成
	ShowWindow(g_hWnd, nCmdShow);	// ウィンドウ表示
	UpdateWindow(g_hWnd);			// ウィンドウ更新

	if (FAILED(InitDevice()))		// デバイスとスワップチェインの生成
	{
		return 0;
	}

	// ポリゴンの初期化
	for (int i = 0; i < SIZE_Y; i++)
	{
		for (int j = 0; j < SIZE_X; j++)
		{
			// 初期化
			g_Polygon[i][j].Init(g_pd3dDevice, "VertexShader.cso", "PixelShader.cso");
			// 色の設定
			g_Polygon[i][j].SetColor(1.0f, 0.0f, 0.0f, 1.0f, 0);
			g_Polygon[i][j].SetColor(1.0f, 0.0f, 0.0f, 1.0f, 1);
			g_Polygon[i][j].SetColor(1.0f, 0.0f, 0.0f, 1.0f, 2);
			g_Polygon[i][j].SetColor(1.0f, 0.0f, 0.0f, 1.0f, 3);
			// 配置
			g_pos[i][j] = XMFLOAT3(j - (SIZE_X / 2.0f) + 0.5f, i - (SIZE_Y / 2.0f) + 0.5f, 0.0f);
			// 落下速度ランダム設定
			g_fallSpeed[i][j] = fallDist(engine);
			// 回転量初期化
			g_rot[i][j] = XMFLOAT3(0.0f, 0.0f, 0.0f);
			// 回転角度ランダム設定
			g_rotSpeed[i][j] = XMFLOAT3(rotDist(engine), rotDist(engine), rotDist(engine));
		}
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

	// 解放
	CleanupDevice();

	return 0;
}

// 描画
void Render()
{
	static bool keyFlg = false;
	// キー入力
	BYTE key[256];
	GetKeyboardState(key);
	if (key[VK_RETURN] & 0x80)
	{
		keyFlg = true;
	}
	// 背景の色を設定 レンダーターゲットクリア
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::Black);
	// 深度/ステンシルバッファクリア
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_World = XMMatrixIdentity();
	//g_World *= XMMatrixScaling(2.0f, 1.0f, 1.0f);
	// 座標変換構造体の変数作成
	for (int i = 0; i < SIZE_Y; i++)
	{
		for (int j = 0; j < SIZE_X; j++)
		{
			g_World2[i][j] = XMMatrixIdentity();
			// キー入力されたら動くよ
			if (keyFlg)
			{
				g_pos[i][j].y += g_fallSpeed[i][j];
				g_World2[i][j] *= XMMatrixRotationX(g_rot[i][j].x += g_rotSpeed[i][j].x);
				g_World2[i][j] *= XMMatrixRotationY(g_rot[i][j].y += g_rotSpeed[i][j].y);
				g_World2[i][j] *= XMMatrixRotationZ(g_rot[i][j].z += g_rotSpeed[i][j].z);
			}
			g_World2[i][j] *= XMMatrixTranslation(g_pos[i][j].x, g_pos[i][j].y, g_pos[i][j].z);
			ConstantBuffer cb;
			// ワールド座標行列を転置
			cb.world = XMMatrixTranspose(g_World2[i][j]);
			// ビュー座標行列を転置
			cb.view = XMMatrixTranspose(g_View);
			// プロジェクション座標行列を転置
			cb.projection = XMMatrixTranspose(g_Proj);
			// データをコピーしてg_pConstantBufferの内容を書き換える（更新する）
			g_pImmediateContext->UpdateSubresource(
				g_pConstantBuffer,	// コピー先リソースへのポインタ
				0,					// コピー先のサブリソースを特定するためのインデックス
				nullptr,			// リソースデータのコピー先となるサブリソースの部分を定義
				&cb,				// コピー元データへのポインタ
				0,					// コピー元データの１行のサイズ
				0					// コピー元データの１深度スライスのサイズ
			);
			g_Polygon[i][j].Render(g_pImmediateContext, g_pConstantBuffer);
		}
	}

	// レンダリングされたイメージを画面に描画する
	g_pSwapChain->Present(0, 0);

}

// 解放
void CleanupDevice()
{
	// ステートのクリア
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
	if (g_pRasterizerState)		g_pRasterizerState->Release();
	if (g_pConstantBuffer)		g_pConstantBuffer->Release();

	for (int i = 0; i < SIZE_Y; i++)
	{
		for (int j = 0; j < SIZE_X; j++)
		{
			g_Polygon[i][j].Release();
		}
	}
}

