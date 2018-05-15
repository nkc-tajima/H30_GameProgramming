#include <tchar.h>			// テキストや文字列を扱うために必要なヘッダーファイル
#include <Windows.h>		// Windowsプログラムで必要になるものが入っている
#include <D3D11.h>			// DirectX11SDKを使うために必要なもの
#include <directxcolors.h>

#pragma comment(lib, "d3d11.lib")	// DirectX11ライブラリを利用するために必要

using namespace std;

// 画面サイズ設定
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// ウィンドウハンドル ウィンドウを操作するために必要なもの

D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
UINT					g_FeatureLevels = 3;
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported;

ID3D11Device*			g_pd3dDevice = nullptr;
ID3D11DeviceContext*	g_pImmediateContext = nullptr;
IDXGISwapChain*			g_pSwapChain = nullptr;
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;
D3D11_VIEWPORT			g_ViewPort;
ID3D11Texture2D*		g_pDepthStencil;
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;

void InitWindow(HINSTANCE hInstance);	 // ウィンドウ生成
HRESULT InitDevice();
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

HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 1;
	desc.BufferDesc.Width = WINDOW_W;
	desc.BufferDesc.Height = WINDOW_H;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.RefreshRate.Numerator = 60;	
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = g_hWnd;	
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Windowed = TRUE;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		g_pFeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&desc,
		&g_pSwapChain,
		&g_pd3dDevice,
		&g_FeatureLevelsSupported,
		&g_pImmediateContext);

	if (FAILED(hr))	return E_FAIL;

	ID3D11Texture2D* pBackBuffer;
	hr = g_pSwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer
	);

	if (FAILED(hr))	return E_FAIL;

	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,
		nullptr,
		&g_pRenderTargetView
	);

	if (FAILED(hr))	return E_FAIL;

	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		nullptr	
	);

	g_ViewPort.TopLeftX = 0.0f;	
	g_ViewPort.TopLeftY = 0.0f;
	g_ViewPort.Width = WINDOW_W;
	g_ViewPort.Height = WINDOW_H;
	g_ViewPort.MinDepth = 0.0f;	
	g_ViewPort.MaxDepth = 1.0f;
	g_pImmediateContext->RSSetViewports(
		1, 
		&g_ViewPort
	);


	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = WINDOW_W;
	descDepth.Height = WINDOW_H;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;	
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(
		&descDepth,
		nullptr,
		&g_pDepthStencil
	);

	if (FAILED(hr))	return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(
		g_pDepthStencil,
		&descDSV,
		&g_pDepthStencilView
	);

	if (FAILED(hr))	return E_FAIL;

	return S_OK;
}
// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// ウィンドウ生成
	ShowWindow(g_hWnd, nCmdShow);	// ウィンドウ表示
	UpdateWindow(g_hWnd);			// ウィンドウ更新

	if (FAILED(InitDevice()))
	{
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

			float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
			g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

			g_pSwapChain->Present(0, 0);
		}
	}

	CleanupDevice();

	return 0;
}

void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}
