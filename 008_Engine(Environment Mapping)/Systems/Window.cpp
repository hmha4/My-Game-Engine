#include "../stdafx.h"
#include "Window.h"
#include "../Program.h"

Program* Window::program = NULL;


WPARAM Window::Run()
{
	MSG msg = { 0 };

	//	D3Ddesc정보를 받아옴
	D3DDesc desc;
	D3D::GetDesc(&desc);

	//	D3D만듬
	D3D::Create();
	DirectWrite::Create();
	Keyboard::Create();
	Mouse::Create();

	Time::Create();
	Time::Get()->Start();

	ImGui::Create(desc.Handle, D3D::GetDevice(), D3D::GetDC());
	//ImGui::StyleColorsLight();
	ImGuiStyleSetting(false, 0.90f);

	program = new Program();
	while (true)
	{
		//	GetMessage 
		//	- 메세지가 없으면 메세지가 생길 때까지 기다린다.
		//	- 하지만 게임에서는 역동적이여야 하기 때문에 GetMessage는 바람직하지 않다.
		//	- 사용자가 어떤 입력을 할때까지 조용히 기다리는 게임은 바라지 않는다.
		//	- 사용자가 입력이 없으면 애니메이션도 안될것이다.
		//	  ==> 메세지가 WM_QUIT이 아니면 무조건 true이고 WM_QUIT이면 false를 반환한다.
		//	PeekMessage
		//	- GetMessage와 비슷하지만 한가지 다른점은 wRemoveMsg이다.
		//	- PM_REMOVE : 메세지 큐에서 메세지를 제거
		//	- PM_NOREMOVE : 메세지 큐에서 메세지를 제거 하지 않음
		//	 -> 대부분의 경우 메세지 큐에서 메세지를 제거한다.
		//	- 메세지가 있으면 true 없으면 false를 반환한다.
		//	  ==> GetMessage처럼 메세지가 들어 올때까지 기다리지 않는다.

		//	메세지를 메세지 큐로부터 읽어 WndProc에 전달
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);	//	키 입력 관련 메세지
			DispatchMessage(&msg);	//	메세지 처리 함수(WinProc)에 메세지 전달
									//	메세지를 점검하여 다음 동작을 결정
		}
		else
		{
			bool check = D3D::Get()->CheckDeviceLost();
			if (check == false) break;

			Time::Get()->Update();

			if (ImGui::IsMouseHoveringAnyWindow() == false)
			{
				Keyboard::Get()->Update();
				Mouse::Get()->Update();
			}

			program->Update();
			ImGui::Update();
			
			program->PreRender();

			D3D::Get()->SetRenderTarget();
			D3D::Get()->Clear();	//	백 버퍼, 깊이/스텐실 버퍼 클리어
			{
				program->Render();
				ImGui::Render();

				DirectWrite::GetDC()->BeginDraw();
				{
					//	BackBuffer를 클리어해 주면 이전게 사라짐
					//	DirectWrite::GetDC()->Clear();
					program->PostRender();
					
				}
				DirectWrite::GetDC()->EndDraw();
			}

			D3D::Get()->Present();	//	후면(백) 버퍼를 전면 버퍼로 시연
			D3D::Get()->SetToNull();
		}
	}
	SAFE_DELETE(program);

	ImGui::Delete();
	Time::Delete();
	Mouse::Delete();
	Keyboard::Delete();
	DirectWrite::Delete();
	D3D::Delete();

	return msg.wParam;
}

LRESULT CALLBACK Window::WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Mouse::Get()->InputProc(msg, wparam, lparam);

	if (ImGui::WndProc((UINT*)handle, msg, wparam, lparam))
		return true;

	//윈도우 화면 크기가 바꼈을 때 영역
	if (msg == WM_SIZE)
	{
		ImGui::Invalidate();
		{
			if (program != NULL)
			{
				float width = (float)LOWORD(lparam);
				float height = (float)HIWORD(lparam);

				if (DirectWrite::Get() != NULL)
					DirectWrite::Get()->DeleteSurface();

				if (D3D::Get() != NULL)
					D3D::Get()->ResizeScreen(width, height);

				if (DirectWrite::Get() != NULL)
					DirectWrite::CreateSurface();

				program->ResizeScreen();
			}
		}
		ImGui::Validate();
	}


	if (msg == WM_CLOSE || msg == WM_DESTROY)
	{
		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
}

Window::Window(int width, int height)
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	//	확장 : 풀스크린 사용
	//	윈도우 클래스 정의
	WNDCLASSEX wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.hInstance = desc.Instance;
	wndClass.lpfnWndProc = (WNDPROC)WinProc;
	wndClass.lpszClassName = desc.AppName.c_str();
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.cbSize = sizeof(WNDCLASSEX);

	//	윈도우 클래스 등록
	WORD wHr = RegisterClassEx(&wndClass);
	assert(wHr != 0);

	//	전체 화면 일때
	if (desc.bFullScreen == true)
	{
		DEVMODE devMode = { 0 };
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmPelsWidth = (DWORD)desc.Width;
		devMode.dmPelsHeight = (DWORD)desc.Height;
		devMode.dmBitsPerPel = 32;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
	}

	//	메모리 상에 윈도우 생성
	desc.Handle = CreateWindowEx
	(
		WS_EX_APPWINDOW
		, desc.AppName.c_str()	//	어떤 윈도우 클래스 사용하는지
		, desc.AppName.c_str()	//	타이틀 바에 뛰울 이름
		, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, NULL
		, (HMENU)NULL
		, desc.Instance
		, NULL
	);
	assert(desc.Handle != NULL);
	D3D::SetDesc(desc);

	//	화면 맞춤
	{
		RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };

		UINT centerX = (GetSystemMetrics(SM_CXSCREEN) - (UINT)desc.Width) / 2;
		UINT centerY = (GetSystemMetrics(SM_CYSCREEN) - (UINT)desc.Height) / 2;

		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
		MoveWindow
		(
			desc.Handle
			, centerX, centerY
			, rect.right - rect.left, rect.bottom - rect.top
			, TRUE
		);
	}

	//	윈도우를 실제로 화면에 띄움
	ShowWindow(desc.Handle, SW_SHOWNORMAL);
	SetForegroundWindow(desc.Handle);
	SetFocus(desc.Handle);

	ShowCursor(true);
}




void Window::ImGuiStyleSetting(bool bStyleDark_, float alpha_)
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowBorderSize = false;
	style.FrameBorderSize = false;
	style.PopupBorderSize = false;
	// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 12.0f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	//style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	if (bStyleDark_)
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			float H, S, V;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

			if (S < 0.1f)
			{
				V = 1.0f - V;
			}
			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			if (col.w < 1.00f)
			{
				col.w *= alpha_;
			}
		}
	}
	else
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			if (col.w < 1.00f)
			{
				col.x *= alpha_;
				col.y *= alpha_;
				col.z *= alpha_;
				col.w *= alpha_;
			}
		}
	}

	ImGuiIO io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Regular.ttf", 15.f, 0, io.Fonts->GetGlyphRangesCyrillic());
}
