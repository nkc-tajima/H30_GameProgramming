#include <tchar.h>			// �e�L�X�g�╶������������߂ɕK�v�ȃw�b�_�[�t�@�C��
#include <Windows.h>		// Windows�v���O�����ŕK�v�ɂȂ���̂������Ă���
#include <D3D11.h>			// DirectX11SDK���g�����߂ɕK�v�Ȃ���
#include <directxcolors.h>

#pragma comment(lib, "d3d11.lib")	// DirectX11���C�u�����𗘗p���邽�߂ɕK�v

using namespace std;

// ��ʃT�C�Y�ݒ�
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// �E�B���h�E�n���h�� �E�B���h�E�𑀍삷�邽�߂ɕK�v�Ȃ���

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

void InitWindow(HINSTANCE hInstance);	 // �E�B���h�E����
HRESULT InitDevice();
void CleanupDevice();

// �E�B���h�E�v���V�[�W���i�R�[���o�b�N�֐��j���������C�x���g���Ƃɏ������s��
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:		// ����{�^���������ꂽ�Ƃ�
		PostMessage(hWnd, WM_DESTROY, 0, 0);	// WM_DESTROY��
		break;
	case WM_DESTROY:	// �E�B���h�E���I������Ƃ�
		PostQuitMessage(0);	// WM_QUIT�𑗐M����
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);	// ����̃E�B���h�E�v���V�[�W�����Ăяo���ď�����Ԃ�
}
// �E�B���h�E����
void InitWindow(HINSTANCE hInstance)
{
	// �E�B���h�E�N���X�o�^
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL002";	// �E�B���h�E�N���X�����ʂ��镶����
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;	// �E�B���h�E�X�^�C��
	wcex.lpfnWndProc = WndProc;				// �E�B���h�E�̃��b�Z�[�W����������R�[���o�b�N�֐��ւ̃|�C���^
	wcex.cbClsExtra = 0;					// �E�B���h�E�N���X�\���̂̌��Ɋ��蓖�Ă�⑫�̃o�C�g��
	wcex.cbWndExtra = 0;					// �E�B���h�E�C���X�^���X�̌��Ɋ��蓖�Ă�⑫�̃o�C�g��
	wcex.hInstance = hInstance;				// ���̃N���X�̂��߂̃E�B���h�E�v���V�[�W��������n���h��
	wcex.hIcon = nullptr;					// �A�C�R��
	wcex.hCursor = nullptr;					// �}�E�X�J�[�\��
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// �E�B���h�E�w�i�F
	wcex.lpszMenuName = nullptr;			// �f�t�H���g���j���[��
	wcex.lpszClassName = szWindowClass;		// �E�B���h�E�N���X��
	wcex.hIconSm = nullptr;					// �������A�C�R��
	RegisterClassEx(&wcex);					// �E�B���h�E�N���X�o�^

	// �E�B���h�E���� �������̓E�B���h�E�n���h���A���s����null���Ԃ�
	g_hWnd = CreateWindow(
		szWindowClass,				// RegisterClass()�œo�^�����N���X�̖��O
		"DirectX11 Tutorial002",	// �^�C�g���o�[
		WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,// �E�B���h�E�X�^�C��
		CW_USEDEFAULT,				// �E�B���h�E����x���W
		CW_USEDEFAULT,				// �E�B���h�E����y���W
		WINDOW_W,					// �E�B���h�E��
		WINDOW_H,					// �E�B���h�E����
		nullptr,					// �e�E�B���h�E�̃n���h��
		nullptr,					// ���j���[�̃n���h��
		wcex.hInstance,				// �E�B���h�E���쐬���郂�W���[���̃n���h��
		nullptr						// WM_CREATE��LPARAM�ɓn�������l
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
// �G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// �E�B���h�E����
	ShowWindow(g_hWnd, nCmdShow);	// �E�B���h�E�\��
	UpdateWindow(g_hWnd);			// �E�B���h�E�X�V

	if (FAILED(InitDevice()))
	{
		return 0;
	}

	MSG hMsg = {};
	ZeroMemory(&hMsg, sizeof(hMsg));// ������
	while (hMsg.message != WM_QUIT)	// �I����������Ă��Ȃ��ԃ��[�v����
	{
		if (PeekMessage(&hMsg, nullptr, 0, 0, PM_REMOVE))// ���b�Z�[�W���擾����B�o���Ȃ��Ă��J��Ԃ��������s��
		{
			TranslateMessage(&hMsg);// �L�[�{�[�h���b�Z�[�W���當�����b�Z�[�W�𐶐�����
			DispatchMessage(&hMsg);	// �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W��n��
		}
		else
		{
			// �`�揈��

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
