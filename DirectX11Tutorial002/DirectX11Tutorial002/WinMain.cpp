#include <tchar.h>			// テキストや文字列を扱うために必要なヘッダーファイル
#include <Windows.h>		// Windowsプログラムで必要になるものが入っている
#include <D3D11.h>			// DirectX11SDKを使うために必要なもの
// DirectX::Colorsを利用するために必要
#include <directxcolors.h>

#pragma comment(lib, "d3d11.lib")	// DirectX11ライブラリを利用するために必要

using namespace std;

// 画面サイズ設定
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// ウィンドウハンドル ウィンドウを操作するために必要なもの

// DirectXのデバイスのターゲット
D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
// ターゲットの要素数
UINT					g_FeatureLevelbs = 3;
// デバイス作成時に返されるバージョン
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported;

// レンダリング及びリソースの作成に使用
ID3D11Device*			g_pd3dDevice = nullptr;
// レンダリングコマンドを生成するデバイスコンテキスト
ID3D11DeviceContext*	g_pImmediateContext = nullptr;
// レンダリングデータを表示前に格納する
IDXGISwapChain*			g_pSwapChain = nullptr;
// レンダリング時にレンダーターゲットのサブリソースを識別
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;
// 描画範囲を設定
D3D11_VIEWPORT			g_ViewPort;
// 深度/ステンシルリソースとして使用
ID3D11Texture2D*		g_pDepthStencil;
// 深度/ステンシルリーソスにアクセスする
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;

// 関数のプロトタイプ宣言
void InitWindow(HINSTANCE hInstance);	 // ウィンドウ生成
// デバイスとスワップチェーンの初期化
HRESULT InitDevice();
// デバイスとスワップチェーンの解放
void CleanupDevice();

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
	return DefWindowProc(hWnd, message, wParam, lParam);	// 既定のウィンドウプロシージャを呼び出して処理を返す
}
// ウィンドウ生成
void InitWindow(HINSTANCE hInstance)
{
	// ウィンドウクラス登録
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL002";	// ウィンドウクラスを識別する文字列
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
		"DirectX11 Tutorial002",	// タイトルバー
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
	// 初期化
	ZeroMemory(&desc, sizeof(desc));
	// バッファの数
	desc.BufferCount = 1;
	// バッファの幅サイズ指定
	desc.BufferDesc.Width = WINDOW_W;
	// バッファの高さサイズ指定
	desc.BufferDesc.Height = WINDOW_H;
	// 画面に出力するフォーマット
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// リフレッシュレートの分母
	desc.BufferDesc.RefreshRate.Numerator = 60;
	// リフレッシュレートの分子
	desc.BufferDesc.RefreshRate.Denominator = 1;
	// 走査線の方向を指定
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	// スケーリングのオンオフ設定
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// リソース作成オプション
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// 出力ウィンドウへの関連付け
	desc.OutputWindow = g_hWnd;	
	// ピクセル単位のマルチサンプリングの数
	desc.SampleDesc.Count = 1;
	// マルチサンプリングのクオリティ
	desc.SampleDesc.Quality = 0;
	// TRUE:ウィンドウモード FALSE:フルスクリーンモード
	desc.Windowed = TRUE;
	// 画面表示の動作のオプション
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// デバイスとスワップチェーンの作成
	hr = D3D11CreateDeviceAndSwapChain(
		// デバイスの作成時に使用するビデオアダプタへのポインタ
		nullptr,
		// 作成するデバイスのドライバ
		D3D_DRIVER_TYPE_HARDWARE,
		// ソフトウェアラスタライザを実装するDLLのハンドル
		nullptr,
		// 有効にするランタイムレイヤー
		0,
		// 作成を試みるバージョンの順序を指定
		g_pFeatureLevels,
		// g_pFeatureLevelsの要素数
		1,
		// SDKのバージョン
		D3D11_SDK_VERSION,
		// スワップチェーンのパラメータのポインタ
		&desc,
		// レンダリングに使用するオブジェクトへのポインタアドレス
		&g_pSwapChain,
		// 作成されたデバイスのオブジェクトへのポインタアドレス
		&g_pd3dDevice,
		// このデバイスでサポートされているバージョンの最初の要素
		&g_FeatureLevelsSupported,
		// デバイスコンテキストのオブジェクトへのポインタアドレス
		&g_pImmediateContext);

	// エラーチェック
	if (FAILED(hr))	return E_FAIL;	
	
	// バックバッファにアクセスをする為の変数
	ID3D11Texture2D* pBackBuffer;

	hr = g_pSwapChain->GetBuffer(
		// バッファのインデックス
		0,
		// バッファにアクセスするためのインターフェイス
		__uuidof(ID3D11Texture2D),
		// バックバッファインターフェイスへのポインタ
		(LPVOID*)&pBackBuffer
	);

	// エラーチェック
	if (FAILED(hr))	return E_FAIL;

	// バックバッファのレンダーターゲットビューを作成
	hr = g_pd3dDevice->CreateRenderTargetView(
		// レンダーターゲットのオブジェクトへのポインタ
		pBackBuffer,
		// レンダーターゲットビューのオブジェクトへのポインタ
		nullptr,
		// レンダーターゲットビューのリソースを受け取る変数
		&g_pRenderTargetView
	);

	// エラーチェック
	if (FAILED(hr))	return E_FAIL;

	// レンダーターゲットを出力マネージャーのレンダーターゲットとして設定
	g_pImmediateContext->OMSetRenderTargets(
		// 設定するレンダーターゲットの数
		1,
		// デバイスに設定するレンダーターゲットの配列のポインタ
		&g_pRenderTargetView,
		// デバイスに設定する深度/ステンシルビューのポインタ
		nullptr	
	);

	// 描画範囲の設定
	// ワールド座標をスクリーン座標に変換するための情報
	
	// 描画範囲の左上X座標 
	g_ViewPort.TopLeftX = 0.0f;
	// 描画範囲の左上Y座標
	g_ViewPort.TopLeftY = 0.0f;
	// 描画範囲の幅
	g_ViewPort.Width = WINDOW_W;
	// 描画範囲の高さ
	g_ViewPort.Height = WINDOW_H;
	// 描画範囲の深度値の最小値
	g_ViewPort.MinDepth = 0.0f;
	// 描画範囲の深度値の最大値
	g_ViewPort.MaxDepth = 1.0f;
	// パイプラインのラスタライザにビューポートを設定
	g_pImmediateContext->RSSetViewports(
		// 設定するビューポートの数
		1, 
		// デバイスに設定するビューポート配列
		&g_ViewPort
	);

	// 深度・ステンシルテクスチャの設定
	D3D11_TEXTURE2D_DESC descDepth;
	// テクスチャの幅
	descDepth.Width = WINDOW_W;
	// テクスチャの高さ
	descDepth.Height = WINDOW_H;
	// ミップマップレベルの最大数
	descDepth.MipLevels = 1;
	// テクスチャの数
	descDepth.ArraySize = 1;
	// フォーマットの設定：深度のみ
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	// ピクセル単位のマルチサンプリングの数
	descDepth.SampleDesc.Count = 1;	
	// マルチサンプリングのクォリティ
	descDepth.SampleDesc.Quality = 0;
	// テクスチャの読み込み方法、書き込み方法の識別
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	// パイプラインステージの設定に関するフラグ
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	// 許可するCPUアクセスのフラグ
	descDepth.CPUAccessFlags = 0;
	// その他のオプション設定
	descDepth.MiscFlags = 0;

	// 深度/ステンシルテクスチャの作成
	hr = g_pd3dDevice->CreateTexture2D(
		// 2Dテクスチャの記述へのポインタ
		&descDepth,
		// サブリソースの記述の配列へのポインタ
		nullptr,
		// 作成されるテクスチャへのポインタのアドレス
		&g_pDepthStencil
	);

	// エラーチェック
	if (FAILED(hr))	return E_FAIL;

	// 深度/ステンシルビューの設定
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	// リソースデータのフォーマットの設定
	descDSV.Format = descDepth.Format;
	// 深度/ステンシルリソースへのアクセス方法を指定
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	// テクスチャが読み取り専用かどうか記述する値
	descDSV.Flags = 0;
	// 最初に使用するミップマップレベルのインデックス
	descDSV.Texture2D.MipSlice = 0;
	// 深度/ステンシルビューの作成
	hr = g_pd3dDevice->CreateDepthStencilView(
		// 深度/ステンシルサーフェスとして機能するリソースへのポインタ
		g_pDepthStencil,
		// 深度/ステンシルビューの記述へのポインタ
		&descDSV,
		// 作成したビューを受け取る
		&g_pDepthStencilView
	);

	// エラーチェック
	if (FAILED(hr))	return E_FAIL;

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
			// 背景色
			float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
			// レンダーターゲットのすべての要素に１つの値を設定
			// 背景の色を設定
			g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

			// レンダリングされたイメージを画面に描画する
			g_pSwapChain->Present(0, 0);
		}
	}
	// デバイスとスワップチェーンの解放
	CleanupDevice();

	return 0;
}

// デバイスとスワップチェーンの解放
void CleanupDevice()
{
	// ステートの解放
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	// インターフェイスの解放
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}
