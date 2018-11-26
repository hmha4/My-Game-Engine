#pragma once

class Window
{
public:
	Window(int width, int height);

	WPARAM Run();

private:
	void ImGuiStyleSetting(bool bStyleDark_, float alpha_);

private:
	static LRESULT CALLBACK WinProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

	static class Program* program;
};