// ---------------------------------------------------------------------------------
// GT3A&GK1A �Q�[���v���O���~���O
// �O�p�`��`��
// ---------------------------------------------------------------------------------
#include <tchar.h>			// �e�L�X�g�╶������������߂ɕK�v�ȃw�b�_�[�t�@�C��
#include <Windows.h>		// Windows�v���O�����ŕK�v�ɂȂ���̂������Ă���
#include <D3D11.h>			// DirectX11SDK���g�����߂ɕK�v�Ȃ���
#include <d3dcompiler.h>
#include <directxcolors.h>	// DirectX::Colors�𗘗p���邽�߂ɕK�v

#pragma comment(lib, "d3d11.lib")		// DirectX11���C�u�����𗘗p���邽�߂ɕK�v
#pragma comment(lib, "d3dCompiler.lib")	// �V�F�[�_�[�R�[�h���R���p�C�����邽�߂ɕK�v

using namespace std;
using namespace DirectX;

// ���_�f�[�^�\����
struct Vertex {
	float pos[3];	// ���W
	float col[4];	// �F
};

// ���̓A�Z���u���A���̓��C�A�E�g
D3D11_INPUT_ELEMENT_DESC g_VertexDesc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

// ��ʃT�C�Y�ݒ�
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// �E�B���h�E�n���h�� �E�B���h�E�𑀍삷�邽�߂ɕK�v�Ȃ���

// DirectX�̃f�o�C�X�̃^�[�Q�b�g
D3D_FEATURE_LEVEL		g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
UINT					g_FeatureLevels = 3;	  // �^�[�Q�b�g�̗v�f��
D3D_FEATURE_LEVEL		g_FeatureLevelsSupported; // �f�o�C�X�쐬���ɕԂ����o�[�W����

// �C���^�[�t�F�C�X
ID3D11Device*			g_pd3dDevice = nullptr;			// �����_�����O����у��\�[�X�̍쐬�Ɏg�p
ID3D11DeviceContext*	g_pImmediateContext = nullptr;	// �����_�����O�R�}���h�𐶐�����f�o�C�X�R���e�L�X�g
IDXGISwapChain*			g_pSwapChain = nullptr;			// �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@���g���ĉ�ʂ��X�V����
ID3D11RenderTargetView*	g_pRenderTargetView = nullptr;	// �����_�����O���Ƀ����_�[�^�[�Q�b�g�̃T�u���\�[�X������
D3D11_VIEWPORT			g_ViewPort;						// �`��͈�
ID3D11Texture2D*		g_pDepthStencil;				// �[�x/�X�e���V�����\�[�X�Ƃ��Ďg�p����
ID3D11DepthStencilView*	g_pDepthStencilView = nullptr;	// �[�x/�X�e���V�����\�[�X�ɃA�N�Z�X����

ID3D11VertexShader*     g_pVertexShader = nullptr;		// ���_�V�F�[�_
ID3D11PixelShader*      g_pPixelShader = nullptr;		// �s�N�Z���V�F�[�_

ID3D11InputLayout*      g_pVertexLayout = nullptr;		// ���̓A�Z���u���X�e�[�W�̓��̓f�[�^�ɃA�N�Z�X
ID3D11Buffer*           g_pVertexBuffer = nullptr;		// ���_�f�[�^��ۑ�����

ID3D11BlendState*		g_pBlendState = nullptr;		// �A���t�@�u�����f�B���O 

// �֐��̃v���g�^�C�v�錾
void InitWindow(HINSTANCE hInstance);	 // �E�B���h�E����
HRESULT InitDevice();		// �f�o�C�X�ƃX���b�v�`�F�[��������
void CleanupDevice();		// �f�o�C�X�ƃX���b�v�`�F�[�����
void Render();				// �`��

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
	// ����̃E�B���h�E�v���V�[�W�����Ăяo���ď�����Ԃ�
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// �V�F�[�_�[�t�@�C����ǂݍ���
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	// �R���p�C����HLSL�R�[�h���R���p�C��������@���w��
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS; // �����ȃR���p�C��������
// �f�o�b�O���[�h�̂Ƃ��̂�
#ifdef _DEBUG
	// �f�o�b�O�����R�[�h�̑}������
	dwShaderFlags |= D3DCOMPILE_DEBUG;			 
	// �R�[�h�������ɍœK���X�e�b�v���X�L�b�v����
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;	 
#endif
	// �R���p�C����̃o�C�i���f�[�^��ۑ�����ϐ�
	ID3DBlob* pErrorBlob = nullptr;	
	// HLSL�R�[�h���o�C�g�R�[�h�ɃR���p�C������
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	// �G���[�`�F�b�N
	if (FAILED(hr))
	{
		// �f�[�^���ۑ�����Ă���H
		if (pErrorBlob)
		{
			// ��������f�o�b�K�ɑ���
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			// ���
			pErrorBlob->Release();
		}
		return hr;
	}
	// �f�[�^�������Ă�����������
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

// �E�B���h�E����
void InitWindow(HINSTANCE hInstance)
{
	// �E�B���h�E�N���X�o�^
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL003";	// �E�B���h�E�N���X�����ʂ��镶����
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
		"DirectX11 Tutorial003",	// �^�C�g���o�[
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
	ZeroMemory(&desc, sizeof(desc));	// ������
	desc.BufferCount = 1;				// �o�b�N�o�b�t�@�̐�
	desc.BufferDesc.Width = WINDOW_W;	// �o�b�N�o�b�t�@�̕�
	desc.BufferDesc.Height = WINDOW_H;	// �o�b�N�o�b�t�@�̍���
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// ��ʂɏo�͂���t�H�[�}�b�g
	desc.BufferDesc.RefreshRate.Numerator = 60;			// ���t���b�V�����[�g�i���q�j
	desc.BufferDesc.RefreshRate.Denominator = 1;		// ���t���b�V�����[�g�i����j
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;// �������̕������w��
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;// �X�P�[�����O�̃I���I�t�ݒ�
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// ���\�[�X�쐬�I�v�V����
	desc.OutputWindow = g_hWnd;		// �o�̓E�B���h�E�ւ̊֘A�t��
	desc.SampleDesc.Count = 1;		// �s�N�Z���P�ʂ̃}���`�T���v�����O�̐�
	desc.SampleDesc.Quality = 0;	// �}���`�T���v�����O�̃N�I���e�B
	desc.Windowed = TRUE;			// TRUE:�E�B���h�E���[�h FALES:�t���X�N���[�����[�h
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ��ʕ\���̓���̃I�v�V����

	// �f�o�C�X�ƃX���b�v�`�F�C���̍쐬
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,					// �f�o�C�X�̍쐬���Ɏg�p����r�f�I�A�_�v�^�ւ̃|�C���^
		D3D_DRIVER_TYPE_HARDWARE,	// �쐬����f�o�C�X�̎�ނ�\�� D3D_DRIVER_TYPE 
		nullptr,					// �\�t�g�E�F�A���X�^���C�U�[����������DLL�̃n���h��
		0,							// �L���ɂ��郉���^�C�����C���[
		g_pFeatureLevels,			// �쐬�����݂�@�\���x���̏������w��
		1,							// pFeatureLevels�̗v�f��
		D3D11_SDK_VERSION,			// SDK�̃o�[�W����
		&desc,						// �X���b�v �`�F�[���̏������p�����[�^�[���i�[����X���b�v�`�F�C���̋L�q�ւ̃|�C���^
		&g_pSwapChain,				// �����_�����O�Ɏg�p����X���b�v�`�F�C����\��IDXGISwapChain�I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X��Ԃ�
		&g_pd3dDevice,				// �쐬���ꂽ�f�o�C�X��\��ID3D11Device�I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X��Ԃ�
		&g_FeatureLevelsSupported,	// ���̃f�o�C�X�ŃT�|�[�g����Ă���@�\���x���̔z��ɂ���ŏ��̗v�f��\��D3D_FEATURE_LEVEL�ւ̃|�C���^��Ԃ�
		&g_pImmediateContext);		// �f�o�C�X �R���e�L�X�g��\��ID3D11DeviceContext�I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X��Ԃ�

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N


	// �o�b�N�o�b�t�@��`��^�[�Q�b�g�ɐݒ�
	// �X���b�v�`�F�C������ŏ��̃o�b�N�o�b�t�@���擾����
	ID3D11Texture2D* pBackBuffer;	// �o�b�t�@�̃A�N�Z�X�Ɏg���C���^�[�t�F�C�X(�o�b�t�@��2D�e�N�X�`���Ƃ��Ĉ���)
	hr = g_pSwapChain->GetBuffer(
		0,							// �o�b�t�@�̃C���f�b�N�X
		__uuidof(ID3D11Texture2D),	// �o�b�t�@�ɃA�N�Z�X����C���^�[�t�F�C�X
		(LPVOID*)&pBackBuffer		// �o�b�N�o�b�t�@ �C���^�[�t�F�C�X�ւ̃|�C���^
	);

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[���쐬
	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,			// �����_�[�^�[�Q�b�g�̃I�u�W�F�N�g�ւ̃|�C���^
		nullptr,				// �����_�[�^�[�Q�b�g�r���[�̃I�u�W�F�N�g�ւ̃|�C���^
		&g_pRenderTargetView	// �����_�[�^�[�Q�b�g�r���[�̃��\�[�X���󂯎��ϐ�
	);

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// �����_�[�^�[�Q�b�g���o�̓}�l�[�W���[�̃����_�[�^�[�Q�b�g�Ƃ��Đݒ�
	g_pImmediateContext->OMSetRenderTargets(
		1,						// �ݒ肷�郌���_�[�^�[�Q�b�g�̐�
		&g_pRenderTargetView,	// �f�o�C�X�ɐݒ肷�郌���_�[�^�[�Q�b�g�̔z��̃|�C���^
		nullptr			        // �f�o�C�X�ɐݒ肷��[�x/�X�e���V���r���[�̃|�C���^
	);

	// �r���[�|�[�g�̐ݒ�(�`��̈�)-1.0�`1.0�͈̔͂ō��ꂽ���[���h���W���X�N���[�����W�ɕϊ����邽�߂̏��
	g_ViewPort.TopLeftX = 0.0f;		// �r���[�|�[�g�̈�̍���X���W
	g_ViewPort.TopLeftY = 0.0f;		// �r���[�|�[�g�̈�̍���Y���W
	g_ViewPort.Width = WINDOW_W;	// �r���[�|�[�g�̈�̕�
	g_ViewPort.Height = WINDOW_H;	// �r���[�|�[�g�̈�̍���
	g_ViewPort.MinDepth = 0.0f;		// �r���[�|�[�g�̈�̐[�x�l�̍ŏ��l
	g_ViewPort.MaxDepth = 1.0f;		// �r���[�|�[�g�̈�̐[�x�l�̍ő�l
	// ���X�^���C�U�Ƀr���[�|�[�g��ݒ�
	g_pImmediateContext->RSSetViewports(
		1,				// �ݒ肷��r���[�|�[�g�̐�
		&g_ViewPort		// �ݒ肷��D3D11_VIEWPORT�\���̂̔z��
	);


	// �[�x/�X�e���V���e�N�X�`���̍쐬
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = WINDOW_W;					// ��
	descDepth.Height = WINDOW_H;				// ����
	descDepth.MipLevels = 1;					// �~�b�v�}�b�v ���x����
	descDepth.ArraySize = 1;					// �z��T�C�Y
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;	// �t�H�[�}�b�g(�[�x�̂�)
	descDepth.SampleDesc.Count = 1;				// �}���`�T���v�����O�̐ݒ�
	descDepth.SampleDesc.Quality = 0;			// �}���`�T���v�����O�̕i��
	descDepth.Usage = D3D11_USAGE_DEFAULT;		// �g�p���@ �f�t�H���g
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// �[�x/�X�e���V���Ƃ��Ďg�p
	descDepth.CPUAccessFlags = 0;				// ������CPU�A�N�Z�X�̃t���O
	descDepth.MiscFlags = 0;					// ���̑��̃I�v�V�����ݒ�

	// 2D�e�N�X�`���[�̔z����쐬
	hr = g_pd3dDevice->CreateTexture2D(
		&descDepth,			// 2D�e�N�X�`���̋L�q�ւ̃|�C���^
		nullptr,			// �T�u���\�[�X�̋L�q�̔z��ւ̃|�C���^
		&g_pDepthStencil	// �쐬�����e�N�X�`���ւ̃|�C���^�̃A�h���X
	);

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// �[�x/�X�e���V���r���[�̍쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;	// ���\�[�X �f�[�^�̃t�H�[�}�b�g
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;// �[�x/�X�e���V�����\�[�X�ւ̃A�N�Z�X���@���w��
	descDSV.Flags = 0;					// �e�N�X�`���[���ǂݎ���p���ǂ���
	descDSV.Texture2D.MipSlice = 0;		// �ŏ��Ɏg�p����~�b�v�}�b�v���x���̃C���f�b�N�X
	
	// �[�x/�X�e���V���r���[�̍쐬
	hr = g_pd3dDevice->CreateDepthStencilView(
		g_pDepthStencil,	// �[�x/�X�e���V���T�[�t�F�X�Ƃ��ċ@�\���郊�\�[�X�ւ̃|�C���^
		&descDSV,			// �[�x/�X�e���V���r���[�̋L�q�ւ̃|�C���^
		&g_pDepthStencilView// �쐬�����r���[���󂯎��
	);

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// �u�����f�B���O�X�e�[�g���L�q
	D3D11_BLEND_DESC descBlend;
	// ������
	ZeroMemory(&descBlend, sizeof(descBlend));
	// �s�N�Z���������_�[�^�[�Q�b�g�ɐݒ肷��Ƃ��A
	// �A���t�@�g�D�J�o���b�W���}���`�T���v�����O�e�N�j�b�N�Ƃ��Ďg�p���邩�ǂ���
	descBlend.AlphaToCoverageEnable = FALSE;	
	// ���������̃����_�[�^�[�Q�b�g�œƗ������u�����f�B���O��
	// �L���ɂ��邩�ǂ���
	descBlend.IndependentBlendEnable = FALSE;	
	// �u�����f�B���O��L���ɂ���
	descBlend.RenderTarget[0].BlendEnable = TRUE;	
	// �ŏ���RGB�f�[�^�\�[�X���w��
	descBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;	
	// 2�Ԗڂ�RGB�f�[�^�\�[�X���w��
	descBlend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	// RGB�f�[�^�\�[�X�̑g�ݍ��킹���@���`
	descBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;		
	// �ŏ��̃A���t�@�f�[�^�\�[�X���w��
	descBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	// 2�Ԗڂ̃A���t�@�f�[�^�\�[�X���w��
	descBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	// �A���t�@�f�[�^�\�[�X�̑g�ݍ��킹���@���`
	descBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	// �������݃}�X�N
	descBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  
	// �u�����f�B���O�X�e�[�g���쐬
	g_pd3dDevice->CreateBlendState(
		// �u�����f�B���O�X�e�[�g�̋L�q�ւ̃|�C���^
		&descBlend,			
		// �쐬�����I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X
		&g_pBlendState	 
	);
	// �u�����f�B���O�W���̔z��
	float blendFactor[4] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ZERO
	};
	// �u�����f�B���O�X�e�[�g�̐ݒ�
	g_pImmediateContext->OMSetBlendState(
		// �u�����f�B���O�X�e�[�g�ւ̃|�C���^
		g_pBlendState, 
		// �u�����f�B���O�W���̔z��ARGBA�������ɂP����
		blendFactor,
		// 32�r�b�g�̃T���v���J�o���b�W
		0xffffffff
	);

	// ���_�V�F�[�_�̃f�[�^��ۑ�
	ID3DBlob* pVSBlob = nullptr;
	// �t�@�C���̃p�X��ݒ�
	wchar_t vs[] = L"VertexShader.hlsl";
	// �V�F�[�_�[�t�@�C����ǂݍ���
	hr = CompileShaderFromFile(
		vs,			// �t�@�C���p�X
		"main",		// �G���g���[�|�C���g�̖��O
		"vs_5_0",	// �V�F�[�_�[�̃o�[�W����
		&pVSBlob	// �ۑ�����ϐ� 
	);

	// ���_�V�F�[�_�̍쐬
	hr = g_pd3dDevice->CreateVertexShader(
		// �R���p�C���ς݃V�F�[�_�ւ̃|�C���^
		pVSBlob->GetBufferPointer(),
		// �R���p�C���ς݃V�F�[�_�̃T�C�Y
		pVSBlob->GetBufferSize(),
		// �N���X�����N�C���^�[�t�F�C�X�ւ̃|�C���^
		nullptr,
		// �V�F�[�_�I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X
		&g_pVertexShader
	);
	
	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// ���̓��C�A�E�g�̍쐬
	hr = g_pd3dDevice->CreateInputLayout(
		// ���̓A�Z���u���X�e�[�W�̓��̓f�[�^�^�̔z��
		g_VertexDesc,
		// ���͗v�f�̔z����̓��̓f�[�^�^�̐�
		ARRAYSIZE(g_VertexDesc),
		// �R���p�C���ς݃V�F�[�_�̃|�C���^
		pVSBlob->GetBufferPointer(),
		// �R���p�C���ς݃V�F�[�_�̃T�C�Y
		pVSBlob->GetBufferSize(),
		// �쐬�������̓��C�A�E�g�I�u�W�F�N�g�ւ̃|�C���^
		&g_pVertexLayout
	);
	// ���
	pVSBlob->Release();
	
	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// �s�N�Z���V�F�[�_�̃f�[�^��ۑ�
	ID3DBlob* pPSBlob = nullptr;
	// �t�@�C���̃p�X��ݒ�
	wchar_t ps[] = L"PixelShader.hlsl";
	// �V�F�[�_�[�t�@�C����ǂݍ���
	hr = CompileShaderFromFile(
		ps,			// �t�@�C���p�X
		"main",		// �G���g���[�|�C���g�̖��O
		"ps_5_0", 	// �V�F�[�_�[�̃o�[�W����
		&pPSBlob	// �ۑ�����ϐ� 
	);

	// �s�N�Z���V�F�[�_�̍쐬
	hr = g_pd3dDevice->CreatePixelShader(
		// �R���p�C���ς݃V�F�[�_�ւ̃|�C���^
		pPSBlob->GetBufferPointer(),
		// �R���p�C���ς݃V�F�[�_�̃T�C�Y
		pPSBlob->GetBufferSize(),
		// �N���X�����N�C���^�[�t�F�C�X�ւ̃|�C���^
		nullptr,
		// �V�F�[�_�I�u�W�F�N�g�ւ̃|�C���^�̃A�h���X
		&g_pPixelShader
	);
	// ���
	pPSBlob->Release();

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

	// ���_���̐ݒ�
	Vertex g_VertexList[]
	{	 	
		//   x���W	y���W z���W		 R	   G	 B	   A
		{ { -0.5f,  0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f } },// 1�ڂ̒��_
		{ {  0.5f, -0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f, 1.0f } },// 2�ڂ̒��_
		{ { -0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f } },// 3�ڂ̒��_
	};
	// �o�b�t�@���\�[�X�ɂ��ċL�q
	D3D11_BUFFER_DESC descBuffer;
	// ������
	ZeroMemory(&descBuffer, sizeof(descBuffer));
	// �o�b�t�@�őz�肳��Ă���ǂݍ��݋y�я������݂̕��@������
	descBuffer.Usage = D3D11_USAGE_DEFAULT;
	// �o�b�t�@�̃T�C�Y ����͎O�p�`��\������̂ŃT�C�Y��3��
	descBuffer.ByteWidth = sizeof(Vertex) * 3;
	// �o�b�t�@���ǂ̂悤�ɃO���t�B�b�N�X�p�C�v���C���Ƀo�C���h���邩������
	descBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPU�̃A�N�Z�X�t���O
	descBuffer.CPUAccessFlags = 0;

	// �T�u���\�[�X�̏������Ɏg�p�����f�[�^���w��
	D3D11_SUBRESOURCE_DATA InitData;
	// ������
	ZeroMemory(&InitData, sizeof(InitData));
	// �������f�[�^�ւ̃|�C���^
	InitData.pSysMem = g_VertexList;
	// �e�N�X�`���ɂ���1�{�̐��̐�[����ׂ̐��܂ł̋���
	InitData.SysMemPitch = 0;
	// 3D�e�N�X�`���Ɋ֘A����l
	InitData.SysMemSlicePitch = 0;

	// �o�b�t�@�̍쐬
	hr = g_pd3dDevice->CreateBuffer(
		// �o�b�t�@�̋L�q�ւ̃|�C���^
		&descBuffer,
		// �������f�[�^�ւ̃|�C���^
		&InitData, 
		// �쐬�����o�b�t�@�ւ̃|�C���^�̃A�h���X
		&g_pVertexBuffer
	);

	if (FAILED(hr))	return E_FAIL;	// �G���[�`�F�b�N

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
			Render();
		}
	}

	// �f�o�C�X�ƃX���b�v�`�F�[���̉��
	CleanupDevice();

	return 0;
}

// �`�揈��
void Render()
{
	// �w�i�̐F��ݒ� �����_�[�^�[�Q�b�g�N���A
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::Black);
	// �[�x/�X�e���V���o�b�t�@�N���A
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ���̓A�Z���u���X�e�[�W�ɓ��̓��C�A�E�g�I�u�W�F�N�g���o�C���h
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	// ���̓A�Z���u���X�e�[�W�ɒ��_�o�b�t�@�̔z����o�C���h
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pImmediateContext->IASetVertexBuffers(
		0,					// �o�C���h�Ɏg�p����ŏ��̓��̓X���b�g
		1,					// �z����̒��_�o�b�t�@�̐�
		&g_pVertexBuffer,	// ���_�o�b�t�@�̔z��ւ̃|�C���^
		&strides,			// �X�g���C�h�l
		&offsets			// �I�t�Z�b�g�l
	);
	// ���_�V�F�[�_���f�o�C�X�ɐݒ�
	g_pImmediateContext->VSSetShader(
		g_pVertexShader,	// ���_�V�F�[�_�ւ̃|�C���^
		nullptr,			// �N���X�C���X�^���X�C���^�[�t�F�C�X�̔z��ւ̃|�C���^
		0					// �z��̃N���X�C���X�^���X�C���^�[�t�F�C�X�̐�
	);
	// �s�N�Z���V�F�[�_���f�o�C�X�ɐݒ�
	g_pImmediateContext->PSSetShader(
		g_pPixelShader,		// �s�N�Z���V�F�[�_�ւ̃|�C���^
		nullptr, 			// �N���X�C���X�^���X�C���^�[�t�F�C�X�̔z��ւ̃|�C���^
		0					// �z��̃N���X�C���X�^���X�C���^�[�t�F�C�X�̐�
	);
	// ���̓A�Z���u���X�e�[�W�Ƀv���~�e�B�u�̌`����o�C���h
	g_pImmediateContext->IASetPrimitiveTopology(
		// ���_�f�[�^���O�p�`�̃��X�g�Ƃ��ĉ���
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		// �����̎O�p�`���o�������Ƃ���TRIANGLESTRIP�ɂ���ƒ��_��ߖ�ł���
	);
	// �v���~�e�B�u��`��
	g_pImmediateContext->Draw(
		3,		// �`�悷�钸�_�̐�
		0		// �ŏ��̒��_�̃C���f�b�N�X
	);
	// �����_�����O���ꂽ�C���[�W����ʂɕ`�悷��
	g_pSwapChain->Present(0, 0);

}

// �f�o�C�X�ƃX���b�v�`�F�[���̉��
void CleanupDevice()
{
	// �X�e�[�g�̉��
	if (g_pImmediateContext)	g_pImmediateContext->ClearState();

	// �C���^�[�t�F�C�X�̉��
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

