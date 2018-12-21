#include "stdafx.h"
#include "Systems/Window.h"

HWND _hWnd;
HINSTANCE _hInstance;

//	Window는 콘솔이랑 다르기 때문에
//	Entity Point가 main이 아니라 WinMain이다.
//	APIENTRY는 __stdcall형 호출 규약을 사용한다는 뜻이다.
//	WinMain은 윈도우를 만들고 화면에 출력마기만 할 뿐
//	대부분의 일은 WinProc에서 이루어진다.
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prvInstance, LPSTR cmdLine, int cmdShow)
{
	//_CrtSetBreakAlloc(1655);
	//	hInstance : 프로그램 인스턴스 핸들
	//				만약 프로그램이 2개 켜져있다면 이걸로 판단
	//	prevInstance : 바로 앞에 실행된 현재프로그램의 인스턴스 핸들
	//					Win32에서는 사용하지 않음
	//	cmdLine : 명령행으로 입력된 인수
	//	cmdShow : 프로그램이 싱행될 형태(최소화, 보통모양 등)
	//	이중 hInstance이외에는 잘 사용 안됨

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
#endif
	//	D3D에 사용될 사용자 정의 구조체
	D3DDesc desc;
	desc.AppName = L"D3D Game";		//	타이틀 명
	desc.Instance = hInstance;		//	프로그램의 식별자(프로그램의 시작주소(ID))
	desc.bFullScreen = false;		//	전체화면 할꺼냐
	desc.bVsync = false;			//	Vsyc 사용할꺼냐
	desc.bStandBy = false;			//	필요없는 화면 렌더링 제어
	desc.Handle = NULL;				//	핸들(윈도우가 만들어지면 값이 들어감)
	desc.Width = 1600;				//	렌더링 영역
	desc.Height = 900;
	D3D::SetDesc(desc);				//	D3D에 값을 넘겨줌

	//	메모리상에 화면 생성
	Window * window = new Window(1600, 900);

	//	메세지 루프
	WPARAM wParam = window->Run();
	SAFE_DELETE(window);

	return wParam;
}