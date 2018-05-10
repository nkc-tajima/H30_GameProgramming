#include <tchar.h>		// �e�L�X�g�╶������������߂ɕK�v�ȃw�b�_�[�t�@�C��
#include <Windows.h>	// Windows�v���O�����ŕK�v�ɂȂ���̂������Ă���
#include <D3D11.h>		// DirectX11SDK���g�����߂ɕK�v�Ȃ���

#pragma comment(lib, "d3d11.lib")	// DirectX11���C�u�����𗘗p���邽�߂ɕK�v

// ��ʃT�C�Y�ݒ�
#define WINDOW_W 1280
#define WINDOW_H 720

HWND g_hWnd;			// �E�B���h�E�n���h�� �E�B���h�E�𑀍삷�邽�߂ɕK�v�Ȃ���

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
	TCHAR szWindowClass[] = "DIRECTX11 TUTORIAL001";	// �E�B���h�E�N���X�����ʂ��镶����
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
		"DirectX11 Tutorial001",	// �^�C�g���o�[
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

// �G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitWindow(hInstance);			// �E�B���h�E����
	ShowWindow(g_hWnd, nCmdShow);	// �E�B���h�E�\��
	UpdateWindow(g_hWnd);			// �E�B���h�E�X�V

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
		}
	}

	return 0;
}