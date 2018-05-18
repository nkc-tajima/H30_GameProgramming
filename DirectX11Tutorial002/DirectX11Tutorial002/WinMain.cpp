#include <tchar.h>			// �e�L�X�g�╶������������߂ɕK�v�ȃw�b�_�[�t�@�C��
#include <Windows.h>		// Windows�v���O�����ŕK�v�ɂȂ���̂������Ă���
#include <D3D11.h>			// DirectX11SDK���g�����߂ɕK�v�Ȃ���
// DirectX::Colors�𗘗p���邽�߂ɕK�v
#include <directxcolors.h>

#pragma comment(lib, "d3d11.lib")	// DirectX11���C�u�����𗘗p���邽�߂ɕK�v

using namespace std;

// ��ʃT�C�Y�ݒ�
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// �E�B���h�E�n���h�� �E�B���h�E�𑀍삷�邽�߂ɕK�v�Ȃ���

// DirectX�̃f�o�C�X�̃^�[�Q�b�g
D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
// �^�[�Q�b�g�̗v�f��
UINT					g_FeatureLevelbs = 3;
// �f�o�C�X�쐬���ɕԂ����o�[�W����
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported;

// �����_�����O�y�у��\�[�X�̍쐬�Ɏg�p
ID3D11Device*			g_pd3dDevice = nullptr;
// �����_�����O�R�}���h�𐶐�����f�o�C�X�R���e�L�X�g
ID3D11DeviceContext*	g_pImmediateContext = nullptr;
// �����_�����O�f�[�^��\���O�Ɋi�[����
IDXGISwapChain*			g_pSwapChain = nullptr;
// �����_�����O���Ƀ����_�[�^�[�Q�b�g�̃T�u���\�[�X������
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;
// �`��͈͂�ݒ�
D3D11_VIEWPORT			g_ViewPort;
// �[�x/�X�e���V�����\�[�X�Ƃ��Ďg�p
ID3D11Texture2D*		g_pDepthStencil;
// �[�x/�X�e���V�����[�\�X�ɃA�N�Z�X����
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;

// �֐��̃v���g�^�C�v�錾
void InitWindow(HINSTANCE hInstance);	 // �E�B���h�E����
// �f�o�C�X�ƃX���b�v�`�F�[���̏�����
HRESULT InitDevice();
// �f�o�C�X�ƃX���b�v�`�F�[���̉��
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
// �f�o�C�X�ƃX���b�v�`�F�[���̏�����
HRESULT InitDevice()
{
	// ������������ɍs��ꂽ�����f����
	HRESULT hr = S_OK;

	// �X���b�v�`�F�[���̐���
	DXGI_SWAP_CHAIN_DESC desc;
	// ������
	ZeroMemory(&desc, sizeof(desc));
	// �o�b�t�@�̐�
	desc.BufferCount = 1;
	// �o�b�t�@�̕��T�C�Y�w��
	desc.BufferDesc.Width = WINDOW_W;
	// �o�b�t�@�̍����T�C�Y�w��
	desc.BufferDesc.Height = WINDOW_H;
	// ��ʂɏo�͂���t�H�[�}�b�g
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// ���t���b�V�����[�g�̕���
	desc.BufferDesc.RefreshRate.Numerator = 60;
	// ���t���b�V�����[�g�̕��q
	desc.BufferDesc.RefreshRate.Denominator = 1;
	// �������̕������w��
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	// �X�P�[�����O�̃I���I�t�ݒ�
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// ���\�[�X�쐬�I�v�V����
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// �o�̓E�B���h�E�ւ̊֘A�t��
	desc.OutputWindow = g_hWnd;	
	// �s�N�Z���P�ʂ̃}���`�T���v�����O�̐�
	desc.SampleDesc.Count = 1;
	// �}���`�T���v�����O�̃N�I���e�B
	desc.SampleDesc.Quality = 0;
	// TRUE:�E�B���h�E���[�h FALSE:�t���X�N���[�����[�h
	desc.Windowed = TRUE;
	// ��ʕ\���̓���̃I�v�V����
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// �f�o�C�X�ƃX���b�v�`�F�[���̍쐬
	hr = D3D11CreateDeviceAndSwapChain(
		// �f�o�C�X�̍쐬���Ɏg�p����r�f�I�A�_�v�^�ւ̃|�C���^
		nullptr,
		// �쐬����f�o�C�X�̃h���C�o
		D3D_DRIVER_TYPE_HARDWARE,
		// �\�t�g�E�F�A���X�^���C�U����������DLL�̃n���h��
		nullptr,
		// �L���ɂ��郉���^�C�����C���[
		0,
		// �쐬�����݂�o�[�W�����̏������w��
		g_pFeatureLevels,
		// g_pFeatureLevels�̗v�f��
		1,
		// SDK�̃o�[�W����
		D3D11_SDK_VERSION,
		// �X���b�v�`�F�[���̃p�����[�^�̃|�C���^
		&desc,
		// �����_�����O�Ɏg�p����I�u�W�F�N�g�ւ̃|�C���^�A�h���X
		&g_pSwapChain,
		// �쐬���ꂽ�f�o�C�X�̃I�u�W�F�N�g�ւ̃|�C���^�A�h���X
		&g_pd3dDevice,
		// ���̃f�o�C�X�ŃT�|�[�g����Ă���o�[�W�����̍ŏ��̗v�f
		&g_FeatureLevelsSupported,
		// �f�o�C�X�R���e�L�X�g�̃I�u�W�F�N�g�ւ̃|�C���^�A�h���X
		&g_pImmediateContext);

	// �G���[�`�F�b�N
	if (FAILED(hr))	return E_FAIL;	
	
	// �o�b�N�o�b�t�@�ɃA�N�Z�X������ׂ̕ϐ�
	ID3D11Texture2D* pBackBuffer;

	hr = g_pSwapChain->GetBuffer(
		// �o�b�t�@�̃C���f�b�N�X
		0,
		// �o�b�t�@�ɃA�N�Z�X���邽�߂̃C���^�[�t�F�C�X
		__uuidof(ID3D11Texture2D),
		// �o�b�N�o�b�t�@�C���^�[�t�F�C�X�ւ̃|�C���^
		(LPVOID*)&pBackBuffer
	);

	// �G���[�`�F�b�N
	if (FAILED(hr))	return E_FAIL;

	// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[���쐬
	hr = g_pd3dDevice->CreateRenderTargetView(
		// �����_�[�^�[�Q�b�g�̃I�u�W�F�N�g�ւ̃|�C���^
		pBackBuffer,
		// �����_�[�^�[�Q�b�g�r���[�̃I�u�W�F�N�g�ւ̃|�C���^
		nullptr,
		// �����_�[�^�[�Q�b�g�r���[�̃��\�[�X���󂯎��ϐ�
		&g_pRenderTargetView
	);

	// �G���[�`�F�b�N
	if (FAILED(hr))	return E_FAIL;

	// �����_�[�^�[�Q�b�g���o�̓}�l�[�W���[�̃����_�[�^�[�Q�b�g�Ƃ��Đݒ�
	g_pImmediateContext->OMSetRenderTargets(
		// �ݒ肷�郌���_�[�^�[�Q�b�g�̐�
		1,
		// �f�o�C�X�ɐݒ肷�郌���_�[�^�[�Q�b�g�̔z��̃|�C���^
		&g_pRenderTargetView,
		// �f�o�C�X�ɐݒ肷��[�x/�X�e���V���r���[�̃|�C���^
		nullptr	
	);

	// �`��͈͂̐ݒ�
	// ���[���h���W���X�N���[�����W�ɕϊ����邽�߂̏��
	
	// �`��͈͂̍���X���W 
	g_ViewPort.TopLeftX = 0.0f;
	// �`��͈͂̍���Y���W
	g_ViewPort.TopLeftY = 0.0f;
	// �`��͈͂̕�
	g_ViewPort.Width = WINDOW_W;
	// �`��͈͂̍���
	g_ViewPort.Height = WINDOW_H;
	// �`��͈͂̐[�x�l�̍ŏ��l
	g_ViewPort.MinDepth = 0.0f;
	// �`��͈͂̐[�x�l�̍ő�l
	g_ViewPort.MaxDepth = 1.0f;
	// �p�C�v���C���̃��X�^���C�U�Ƀr���[�|�[�g��ݒ�
	g_pImmediateContext->RSSetViewports(
		// �ݒ肷��r���[�|�[�g�̐�
		1, 
		// �f�o�C�X�ɐݒ肷��r���[�|�[�g�z��
		&g_ViewPort
	);

	// �[�x�E�X�e���V���e�N�X�`���̐ݒ�
	D3D11_TEXTURE2D_DESC descDepth;
	// �e�N�X�`���̕�
	descDepth.Width = WINDOW_W;
	// �e�N�X�`���̍���
	descDepth.Height = WINDOW_H;
	// �~�b�v�}�b�v���x���̍ő吔
	descDepth.MipLevels = 1;
	// �e�N�X�`���̐�
	descDepth.ArraySize = 1;
	// �t�H�[�}�b�g�̐ݒ�F�[�x�̂�
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	// �s�N�Z���P�ʂ̃}���`�T���v�����O�̐�
	descDepth.SampleDesc.Count = 1;	
	// �}���`�T���v�����O�̃N�H���e�B
	descDepth.SampleDesc.Quality = 0;
	// �e�N�X�`���̓ǂݍ��ݕ��@�A�������ݕ��@�̎���
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	// �p�C�v���C���X�e�[�W�̐ݒ�Ɋւ���t���O
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	// ������CPU�A�N�Z�X�̃t���O
	descDepth.CPUAccessFlags = 0;
	// ���̑��̃I�v�V�����ݒ�
	descDepth.MiscFlags = 0;

	// �[�x/�X�e���V���e�N�X�`���̍쐬
	hr = g_pd3dDevice->CreateTexture2D(
		// 2D�e�N�X�`���̋L�q�ւ̃|�C���^
		&descDepth,
		// �T�u���\�[�X�̋L�q�̔z��ւ̃|�C���^
		nullptr,
		// �쐬�����e�N�X�`���ւ̃|�C���^�̃A�h���X
		&g_pDepthStencil
	);

	// �G���[�`�F�b�N
	if (FAILED(hr))	return E_FAIL;

	// �[�x/�X�e���V���r���[�̐ݒ�
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	// ���\�[�X�f�[�^�̃t�H�[�}�b�g�̐ݒ�
	descDSV.Format = descDepth.Format;
	// �[�x/�X�e���V�����\�[�X�ւ̃A�N�Z�X���@���w��
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	// �e�N�X�`�����ǂݎ���p���ǂ����L�q����l
	descDSV.Flags = 0;
	// �ŏ��Ɏg�p����~�b�v�}�b�v���x���̃C���f�b�N�X
	descDSV.Texture2D.MipSlice = 0;
	// �[�x/�X�e���V���r���[�̍쐬
	hr = g_pd3dDevice->CreateDepthStencilView(
		// �[�x/�X�e���V���T�[�t�F�X�Ƃ��ċ@�\���郊�\�[�X�ւ̃|�C���^
		g_pDepthStencil,
		// �[�x/�X�e���V���r���[�̋L�q�ւ̃|�C���^
		&descDSV,
		// �쐬�����r���[���󂯎��
		&g_pDepthStencilView
	);

	// �G���[�`�F�b�N
	if (FAILED(hr))	return E_FAIL;

	return S_OK;
}
// �G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// �E�B���h�E����
	ShowWindow(g_hWnd, nCmdShow);	// �E�B���h�E�\��
	UpdateWindow(g_hWnd);			// �E�B���h�E�X�V

	// �f�o�C�X�ƃX���b�v�`�F�[���̏�����
	if (FAILED(InitDevice()))		
	{
		// ���s������I��
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
			// �w�i�F
			float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
			// �����_�[�^�[�Q�b�g�̂��ׂĂ̗v�f�ɂP�̒l��ݒ�
			// �w�i�̐F��ݒ�
			g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

			// �����_�����O���ꂽ�C���[�W����ʂɕ`�悷��
			g_pSwapChain->Present(0, 0);
		}
	}
	// �f�o�C�X�ƃX���b�v�`�F�[���̉��
	CleanupDevice();

	return 0;
}

// �f�o�C�X�ƃX���b�v�`�F�[���̉��
void CleanupDevice()
{
	// �X�e�[�g�̉��
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	// �C���^�[�t�F�C�X�̉��
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}
