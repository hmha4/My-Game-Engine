#pragma once

class Window
{
public:
	static WPARAM Run(class IExecute* main);
	

private:
	//	Create Window and Device
	static void Create();
	static void Destroy();
	static void ProgressRender();
	static void MainRender();

	static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

	static void ImGuiStyleSetting(bool bStyleDark_, float alpha_);

private:
	static class IExecute* mainExecute;

	static bool bInitialize;
	static class mutex * m;
};
