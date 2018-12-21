#include "stdafx.h"
#include "Systems/Window.h"

HWND _hWnd;
HINSTANCE _hInstance;

//	Window�� �ܼ��̶� �ٸ��� ������
//	Entity Point�� main�� �ƴ϶� WinMain�̴�.
//	APIENTRY�� __stdcall�� ȣ�� �Ծ��� ����Ѵٴ� ���̴�.
//	WinMain�� �����츦 ����� ȭ�鿡 ��¸��⸸ �� ��
//	��κ��� ���� WinProc���� �̷������.
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prvInstance, LPSTR cmdLine, int cmdShow)
{
	//_CrtSetBreakAlloc(1655);
	//	hInstance : ���α׷� �ν��Ͻ� �ڵ�
	//				���� ���α׷��� 2�� �����ִٸ� �̰ɷ� �Ǵ�
	//	prevInstance : �ٷ� �տ� ����� �������α׷��� �ν��Ͻ� �ڵ�
	//					Win32������ ������� ����
	//	cmdLine : ��������� �Էµ� �μ�
	//	cmdShow : ���α׷��� ����� ����(�ּ�ȭ, ������ ��)
	//	���� hInstance�̿ܿ��� �� ��� �ȵ�

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
#endif
	//	D3D�� ���� ����� ���� ����ü
	D3DDesc desc;
	desc.AppName = L"D3D Game";		//	Ÿ��Ʋ ��
	desc.Instance = hInstance;		//	���α׷��� �ĺ���(���α׷��� �����ּ�(ID))
	desc.bFullScreen = false;		//	��üȭ�� �Ҳ���
	desc.bVsync = false;			//	Vsyc ����Ҳ���
	desc.bStandBy = false;			//	�ʿ���� ȭ�� ������ ����
	desc.Handle = NULL;				//	�ڵ�(�����찡 ��������� ���� ��)
	desc.Width = 1600;				//	������ ����
	desc.Height = 900;
	D3D::SetDesc(desc);				//	D3D�� ���� �Ѱ���

	//	�޸𸮻� ȭ�� ����
	Window * window = new Window(1600, 900);

	//	�޼��� ����
	WPARAM wParam = window->Run();
	SAFE_DELETE(window);

	return wParam;
}