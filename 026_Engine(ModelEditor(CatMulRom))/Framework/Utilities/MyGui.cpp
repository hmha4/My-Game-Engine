#include "Framework.h"
#include "MyGui.h"
#include "Environments/ScatterSky.h"

bool MyGui::show_app_help = false;
bool MyGui::show_app_style = false;
bool MyGui::show_app_sky = false;
bool MyGui::show_app_light = false;
bool MyGui::show_app_camera = false;
bool MyGui::isFreedom = true;
bool MyGui::show_app_inspector = false;
bool MyGui::show_app_hierarchy = false;
bool MyGui::show_app_animation = false;
bool MyGui::show_app_behaviour = false;
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

void MyGui::ShowLightSetting(class Context * context)
{
	if (ImGui::Begin("Light Setting", &show_app_light))
	{
		ImGui::Text("Directional Light");
		ImGui::Separator();
		{
			bool check = false;
			check |= ImGui::DragFloat3("Direction", (float*)&context->GetGlobalLight()->Direction, 0.001f, -1, 1);
			check |= ImGui::ColorEdit3("Ambient", (float*)&context->GetGlobalLight()->Ambient);
			check |= ImGui::ColorEdit3("Diffuse", (float*)&context->GetGlobalLight()->Diffuse);
			check |= ImGui::ColorEdit3("Specular", (float*)&context->GetGlobalLight()->Specular);
			check |= ImGui::DragFloat("Shininess", &context->GetGlobalLight()->Specular.a, 0.01f, 0.0f, 20.0f);

			if (check == true)
				context->ChangeGlobalLight();
		}
		ImGui::Separator();

		ImGui::End();
	}
}

void MyGui::ShowCameraSetting(class Context * values)
{
	if (ImGui::Begin("Camera Setting", &show_app_camera))
	{
		if (ImGui::RadioButton("Freedom Camera", isFreedom))
		{
			isFreedom = true;
			values->GetMainCamera()->CameraType(L"Freedom");
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Orbit Camera", !isFreedom))
		{
			isFreedom = false;
			values->GetMainCamera()->CameraType(L"Orbit");
		}

		ImGui::Spacing();
		ImGui::Separator(); ImGui::Separator();
		ImGui::Spacing(); ImGui::Spacing();

		D3DXVECTOR3 position;
		D3DXVECTOR2 rotation;
		values->GetMainCamera()->Position(&position);
		values->GetMainCamera()->Rotation(&rotation);

		ImGui::Text("Position"); ImGui::SameLine();
		ImGui::Text("x : %.2f y : %.2f z : %.2f", position.x, position.y, position.z);

		ImGui::Text("Rotation"); ImGui::SameLine();
		ImGui::Text("x : %.2f y : %.2f", rotation.x, rotation.y);

		ImGui::Spacing();
		float speed = values->GetMainCamera()->Speed();
		ImGui::DragFloat("Speed", &speed);
		values->GetMainCamera()->Speed(speed);
		ImGui::Separator();

		ImGui::End();
	}
}

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

void MyGui::ShowSkySetting(ScatterSky * sky)
{
	ImGui::Text("Sky Inspector");
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	{
		if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat("StartTheta", &sky->StartTheta(), 0.01f, 0.0f, Math::PI_2);
			ImGui::DragFloat("Theta", &sky->Theta(), 0.01f, 0.0f, Math::PI_2);
			ImGui::DragFloat("Angle", &sky->Leans(), 0.1f, 0.0f, 90.0f);
			ImGui::DragFloat("TimeFactor", &sky->TimeFactor(), 0.01f, 0.0f, 0.5f);
		}

		if (ImGui::CollapsingHeader("Cloud", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::ColorEdit3("Color ", (float*)sky->GetWaveLength());
			ImGui::SliderInt("Sample Count", &sky->GetSampleCount(), 0, 20);
		}

		if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Columns(4);
			ImGui::Text("Star"); ImGui::NextColumn();
			ImGui::Text("Moon"); ImGui::NextColumn();
			ImGui::Text("Glow"); ImGui::NextColumn();
			ImGui::Text("Cloud"); ImGui::NextColumn();
			ImGui::Separator();

			if (ImGui::ImageButton(sky->GetStarFieldMap()->SRV(), ImVec2(50, 50)))
				Path::OpenFileDialog(bind(&MyGui::ChangeTexture, placeholders::_1, sky, 0));
			ImGui::NextColumn();
			if (ImGui::ImageButton(sky->GetMoonMap()->SRV(), ImVec2(50, 50)))
				Path::OpenFileDialog(bind(&MyGui::ChangeTexture, placeholders::_1, sky, 1));
			ImGui::NextColumn();
			if (ImGui::ImageButton(sky->GetGlowMap()->SRV(), ImVec2(50, 50)))
				Path::OpenFileDialog(bind(&MyGui::ChangeTexture, placeholders::_1, sky, 2));
			ImGui::NextColumn();
			if (ImGui::ImageButton(sky->GetCloudMap()->SRV(), ImVec2(50, 50)))
				Path::OpenFileDialog(bind(&MyGui::ChangeTexture, placeholders::_1, sky, 3));
			ImGui::Columns(1);
		}

		ImGui::Separator();
		ImGui::Columns(2);
		ImGui::Text("Mie Color"); ImGui::NextColumn();
		ImGui::Text("Rayleigh Color"); ImGui::NextColumn();
		ImGui::Separator();
		float width = ImGui::GetWindowContentRegionWidth() * 0.5f - 10;
		ImGui::Image(sky->MieSRV(), ImVec2(width, width - 60)); ImGui::NextColumn();
		ImGui::Image(sky->RayleighSRV(), ImVec2(width, width - 60)); ImGui::NextColumn();
		ImGui::Columns(1);
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
}

void MyGui::DrawLog()
{
	log.Draw("Debug Log", &show_app_debugLog);
}

void MyGui::ChangeTexture(wstring fileName, ScatterSky * sky, UINT type)
{
	switch (type)
	{
	case 0:
	{
		SAFE_DELETE(sky->GetStarFieldMap());

		sky->GetStarFieldMap() = new Texture(fileName);
		sky->GetEffect()->AsSRV("StarfieldMap")->SetResource(sky->GetStarFieldMap()->SRV());
	}
	break;
	case 1:
	{
		SAFE_DELETE(sky->GetMoonMap());

		sky->GetMoonMap() = new Texture(fileName);
		sky->GetEffect()->AsSRV("MoonMap")->SetResource(sky->GetMoonMap()->SRV());
	}
	break;
	case 2:
	{
		SAFE_DELETE(sky->GetGlowMap());

		sky->GetGlowMap() = new Texture(fileName);
		sky->GetEffect()->AsSRV("MoonGlowMap")->SetResource(sky->GetGlowMap()->SRV());
	}
	break;
	case 3:
	{
		SAFE_DELETE(sky->GetCloudMap());

		int perm[] = { 151,160,137,91,90,15,
			131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
			88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
			77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
			102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
			5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
			223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
			129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
			49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
		};

		int gradValues[] = {
			1,1,0,
			-1,1,0, 1,-1,0,
			-1,-1,0, 1,0,1,
			-1,0,1, 1,0,-1,
			-1,0,-1, 0,1,1,
			0,-1,1, 0,1,-1,
			0,-1,-1, 1,1,0,
			0,-1,1, -1,1,0,
			0,-1,-1
		};

		sky->GetCloudMap() = new Texture(fileName);

		vector<D3DXCOLOR> pixels;
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 256; j++)
			{
				//int offset = (i * 256 + j);
				int value = perm[(j + perm[i]) & 0xFF];
				D3DXCOLOR color;
				color.r = (float)(gradValues[value & 0x0F] * 64 + 64);
				color.g = (float)(gradValues[value & 0x0F + 1] * 64 + 64);
				color.b = (float)(gradValues[value & 0x0F + 2] * 64 + 64);
				color.a = (float)value;
				pixels.push_back(color);
			}
		}
		sky->GetCloudMap()->WritePixels(DXGI_FORMAT_R8G8B8A8_UNORM, pixels, true);
		sky->GetEffect()->AsSRV("CloudMap")->SetResource(sky->GetCloudMap()->SRV());
	}
	break;
	}

	sky->SetTextures(fileName, type);
}

void MyGui::EditTransform(const float * cameraView, float * cameraProjection, float * matrix)
{
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
	static bool useSnap = false;
	static float snap[3] = { 1.f, 1.f, 1.f };
	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
	static bool boundSizing = false;
	static bool boundSizingSnap = false;

	if (Keyboard::Get()->Down('1'))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (Keyboard::Get()->Down('2'))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (Keyboard::Get()->Down('3')) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation, 3);
	ImGui::InputFloat3("Rt", matrixRotation, 3);
	ImGui::InputFloat3("Sc", matrixScale, 3);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}
	if (Keyboard::Get()->Down('Z'))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();

	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &snap[0]);
		break;
	case ImGuizmo::ROTATE:
		ImGui::InputFloat("Angle Snap", &snap[0]);
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &snap[0]);
		break;
	}
	ImGui::Checkbox("Bound Sizing", &boundSizing);
	if (boundSizing)
	{
		ImGui::PushID(3);
		ImGui::Checkbox("", &boundSizingSnap);
		ImGui::SameLine();
		ImGui::InputFloat3("Snap", boundsSnap);
		ImGui::PopID();
	}

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
}