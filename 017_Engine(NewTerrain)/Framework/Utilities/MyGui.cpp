#include "Framework.h"
#include "MyGui.h"

bool MyGui::show_app_help = false;
bool MyGui::show_app_style = false;
bool MyGui::show_app_sky = false;
bool MyGui::show_app_light = false;
bool MyGui::show_app_camera = false;
bool MyGui::isFreedom = true;
bool MyGui::show_app_inspector = false;
bool MyGui::show_app_hierarchy = false;
bool MyGui::show_app_animation = false;
bool MyGui::show_app_debugLog = false;
bool MyGui::show_app_modal = false;
bool MyGui::show_app_retarget = false;
MyGui::ExampleAppLog MyGui::log;

void MyGui::ShowHelpMarker(const char * desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void MyGui::ShowUserGuide()
{
	if (ImGui::Begin("Guide", &show_app_help))
	{
		ImGui::ShowUserGuide();

		ImGui::End();
	}
}

void MyGui::ShowStyleEdit()
{
	if (ImGui::Begin("Style Editor", &show_app_style))
	{
		ImGui::ShowStyleEditor();

		ImGui::End();
	}
}

void MyGui::ShowLightSetting(float * pos, float * dir)
{
	if (ImGui::Begin("Light Setting", &show_app_light))
	{
		ImGui::Text("Directional Light");
		ImGui::Separator();
		{
			ImGui::DragFloat3("Position", pos, 0.1f, -100, 1000);
			ImGui::DragFloat3("Direction", dir, 0.001f, -1, 1);
		}
		ImGui::Separator();

		ImGui::End();
	}
}
//
//void MyGui::ShowCameraSetting(struct ExecuteValues * values)
//{
//	if (ImGui::Begin("Camera Setting", &show_app_camera))
//	{
//		if (ImGui::RadioButton("Freedom Camera", isFreedom))
//		{
//			isFreedom = true;
//			values->MainCamera->CameraType(L"Freedom");
//		}
//		ImGui::SameLine();
//		if (ImGui::RadioButton("Orbit Camera", !isFreedom))
//		{
//			isFreedom = false;
//			values->MainCamera->CameraType(L"Orbit");
//		}
//
//		ImGui::Spacing();
//		ImGui::Separator(); ImGui::Separator();
//		ImGui::Spacing(); ImGui::Spacing();
//
//		D3DXVECTOR3 position;
//		D3DXVECTOR2 rotation;
//		values->MainCamera->Position(&position);
//		values->MainCamera->Rotation(&rotation);
//
//		ImGui::Text("Position"); ImGui::SameLine();
//		ImGui::Text("x : %.2f y : %.2f z : %.2f", position.x, position.y, position.z);
//
//		ImGui::Text("Rotation"); ImGui::SameLine();
//		ImGui::Text("x : %.2f y : %.2f", rotation.x, rotation.y);
//
//		ImGui::Spacing();
//		float speed = values->MainCamera->Speed();
//		ImGui::DragFloat("Speed", &speed);
//		values->MainCamera->Speed(speed);
//		ImGui::Separator();
//
//
//		ImGui::End();
//	}
//}

void MyGui::OpenPopupModal(const char * str)
{
	if (ImGui::BeginPopupModal("Error"))
	{
		ImGui::Text(str);

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void MyGui::ShowDebugOverlay()
{
	const float DISTANCE = 10.0f;
	static int corner = 0;
	ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE * 3 : DISTANCE * 3);
	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
	if (corner != -1)
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
	if (ImGui::Begin("Statistics", NULL, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::LabelText("Graphics", "%.1fFPS", Time::Get()->FPS());
		if (ImGui::IsItemHovered())
		{
			FPSPlot();
		}
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			//if (p_open && ImGui::MenuItem("Close")) *p_open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

float MyGui::values[90] = { 0 };
int MyGui::values_offset = 0;
double MyGui::refresh_time = 0.0f;

void MyGui::FPSPlot()
{
	ImGui::BeginTooltip();
	{
		if (refresh_time == 0.0f)
			refresh_time = ImGui::GetTime();
		while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
		{
			static float phase = 0.0f;
			values[values_offset] = Time::Get()->FPS();
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			phase += 0.10f*values_offset;
			refresh_time += 1.0f / 60.0f;
		}
		ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, "FPS", 0.0f, 10000.0f, ImVec2(0, 0));
	}
	ImGui::EndTooltip();
}

void MyGui::ShowSkySetting(D3DXCOLOR& centerColor, D3DXCOLOR& apexColor, float& height)
{
	ImGui::Begin("Sky", &show_app_sky);
	{
		ImVec4 color = ImVec4(0, 0, 1, 1);
		ImGui::Separator();
		ImGui::TextColored(color, "Color");
		ImGui::Separator();
		ImGui::ColorEdit4("Center", (float*)centerColor);
		ImGui::ColorEdit4("Apex", (float*)apexColor);
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextColored(color, "Intensity");
		ImGui::Separator();
		ImGui::DragFloat("Height", &height, 0.5f, 1.0f, 10.0f);
	}
	ImGui::End();
}

void MyGui::DrawLog()
{
	log.Draw("Debug Log", &show_app_debugLog);
}

//void MyGui::EditTransform(const float * cameraView, float * cameraProjection, float * matrix)
//{
//	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
//	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
//	static bool useSnap = false;
//	static float snap[3] = { 1.f, 1.f, 1.f };
//	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
//	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
//	static bool boundSizing = false;
//	static bool boundSizingSnap = false;
//
//	if (Keyboard::Get()->Down('1'))
//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//	if (Keyboard::Get()->Down('2'))
//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
//	if (Keyboard::Get()->Down('3')) // r Key
//		mCurrentGizmoOperation = ImGuizmo::SCALE;
//	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//	ImGui::SameLine();
//	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
//	ImGui::SameLine();
//	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
//		mCurrentGizmoOperation = ImGuizmo::SCALE;
//	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
//	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
//	ImGui::InputFloat3("Tr", matrixTranslation, 3);
//	ImGui::InputFloat3("Rt", matrixRotation, 3);
//	ImGui::InputFloat3("Sc", matrixScale, 3);
//	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);
//
//	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
//	{
//		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
//			mCurrentGizmoMode = ImGuizmo::LOCAL;
//		ImGui::SameLine();
//		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
//			mCurrentGizmoMode = ImGuizmo::WORLD;
//	}
//	if (Keyboard::Get()->Down('Z'))
//		useSnap = !useSnap;
//	ImGui::Checkbox("", &useSnap);
//	ImGui::SameLine();
//
//	switch (mCurrentGizmoOperation)
//	{
//	case ImGuizmo::TRANSLATE:
//		ImGui::InputFloat3("Snap", &snap[0]);
//		break;
//	case ImGuizmo::ROTATE:
//		ImGui::InputFloat("Angle Snap", &snap[0]);
//		break;
//	case ImGuizmo::SCALE:
//		ImGui::InputFloat("Scale Snap", &snap[0]);
//		break;
//	}
//	ImGui::Checkbox("Bound Sizing", &boundSizing);
//	if (boundSizing)
//	{
//		ImGui::PushID(3);
//		ImGui::Checkbox("", &boundSizingSnap);
//		ImGui::SameLine();
//		ImGui::InputFloat3("Snap", boundsSnap);
//		ImGui::PopID();
//	}
//
//	ImGuiIO& io = ImGui::GetIO();
//	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
//	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
//}
