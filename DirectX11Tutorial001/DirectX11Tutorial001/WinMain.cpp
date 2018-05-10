#include <tchar.h>		// テキストや文字列を扱うために必要なヘッダーファイル
#include <Windows.h>	// Windowsプログラムで必要になるものが入っている
#include <D3D11.h>		// DirectX11SDKを使うために必要なもの

#pragma comment(lib, "d3d11.lib")	// DirectX11ライブラリを利用するために必要

// 画面サイズ設定
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// ウィンドウハンドル ウィンドウを操作するために必要なもの

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
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL001";	// ウィンドウクラスを識別する文字列
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
		"DirectX11 Tutorial001",	// タイトルバー
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

// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// ウィンドウ生成
	ShowWindow(g_hWnd, nCmdShow);	// ウィンドウ表示
	UpdateWindow(g_hWnd);			// ウィンドウ更新

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
		}
	}

	return 0;
}