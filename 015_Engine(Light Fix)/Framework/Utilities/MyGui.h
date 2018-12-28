#pragma once

class MyGui
{
public:
	static void ShowHelpMarker(const char* desc);
	static void ShowUserGuide();
	static void ShowStyleEdit();
	static void ShowDebugOverlay();
	static void FPSPlot();
	static void ShowSkySetting(D3DXCOLOR& centerColor, D3DXCOLOR& apexColor, float& height);
	static void ShowLightSetting(float * pos, float * dir);
	//static void ShowCameraSetting(struct ExecuteValues * values);
	static void OpenPopupModal(const char * str);
	static void DrawLog();
	static void EditTransform(const float *cameraView, float *cameraProjection, float* matrix);

public:
	static bool show_app_help;
	static bool show_app_style;
	static bool show_app_sky;
	static bool show_app_light;
	static bool show_app_camera;
	static bool show_app_inspector;
	static bool show_app_hierarchy;
	static bool show_app_animation;
	static bool show_app_debugLog;
	static bool show_app_modal;
	static bool show_app_retarget;

private:
	static float values[90];
	static int values_offset;
	static double refresh_time;

	static bool isFreedom;
public:
	struct ExampleAppLog
	{
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets;        // Index to lines offset
		bool                ScrollToBottom;

		void    Clear() { Buf.clear(); LineOffsets.clear(); }

		void    AddLog(const char* fmt, ...) IM_FMTARGS(1)
		{
			int old_size = Buf.size();
			va_list args;
			va_start(args, fmt);
			Buf.appendfv(fmt, args);
			va_end(args);
			for (int new_size = Buf.size(); old_size < new_size; old_size++)
				if (Buf[old_size] == '\n')
					LineOffsets.push_back(old_size);
			ScrollToBottom = true;
		}

		void    Draw(const char* title, bool* p_open = NULL)
		{
			ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
			if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_NoMove))
			{
				ImGui::End();
				return;
			}
			if (ImGui::Button("Clear")) Clear();
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			Filter.Draw("Filter", -100.0f);
			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			if (copy) ImGui::LogToClipboard();

			if (Filter.IsActive())
			{
				const char* buf_begin = Buf.begin();
				const char* line = buf_begin;
				for (int line_no = 0; line != NULL; line_no++)
				{
					const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
					if (Filter.PassFilter(line, line_end))
						ImGui::TextUnformatted(line, line_end);
					line = line_end && line_end[1] ? line_end + 1 : NULL;
				}
			}
			else
			{
				ImGui::TextUnformatted(Buf.begin());
			}

			if (ScrollToBottom)
				ImGui::SetScrollHere(1.0f);
			ScrollToBottom = false;
			ImGui::EndChild();
			ImGui::End();
		}
	};

	static ExampleAppLog log;
};

