#include "stdafx.h"
#include "ModelTool.h"

#include "Environments/Shadow.h"
#include "Models/ModelMeshPart.h"

ModelTool::ModelTool()
{
}

ModelTool::~ModelTool()
{
}

void ModelTool::Initialize()
{
	MyGui::show_app_inspector = true;
	MyGui::show_app_debugLog = true;
	MyGui::show_app_hierarchy = true;
	MyGui::show_app_animation = true;

	object = NULL;
	model = NULL;
	boneIndex = -1;
	meshIndex = -1;
	materialIndex = -1;
	target = TARGET::DEFAULT;
}

void ModelTool::Ready()
{
}

void ModelTool::Update()
{
}

void ModelTool::PreRender()
{
}

void ModelTool::Render()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
			}
			if (ImGui::MenuItem("Save"))
			{
			}
			if (ImGui::MenuItem("Exit"))
			{
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Style", NULL, &MyGui::show_app_style);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			//ImGui::MenuItem("Sky", NULL, &MyGui::show_app_sky);
			ImGui::MenuItem("Light", NULL, &MyGui::show_app_light);
			ImGui::MenuItem("Camera", NULL, &MyGui::show_app_camera);
			ImGui::MenuItem("Inspector", NULL, &MyGui::show_app_inspector);
			ImGui::MenuItem("Hierarchy", NULL, &MyGui::show_app_hierarchy);
			ImGui::MenuItem("Animation", NULL, &MyGui::show_app_animation);
			ImGui::MenuItem("Debug Log", NULL, &MyGui::show_app_debugLog);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("help", NULL, &MyGui::show_app_help);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (MyGui::show_app_help) { MyGui::ShowUserGuide(); }
	if (MyGui::show_app_style) { MyGui::ShowStyleEdit(); }
	//if (MyGui::show_app_sky) { MyGui::ShowSkySetting(sky); }
	if (MyGui::show_app_camera) { MyGui::ShowCameraSetting(Context::Get()); }
	if (MyGui::show_app_light) { MyGui::ShowLightSetting(Context::Get()); }
	if (MyGui::show_app_debugLog) { MyGui::DrawLog(); }

	if (MyGui::show_app_hierarchy) { RenderHeirarchy(); }
	if (MyGui::show_app_inspector) { RenderInspector(); }
	if (MyGui::show_app_animation) { RenderAnimation(); }
}

void ModelTool::Delete()
{
}

void ModelTool::RenderHeirarchy()
{
	ImGui::Begin("Hierarchy", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
	{
		//	Heirarchy Filter
		if (filter.Draw())
		{
			ImGui::Text("");
		}

		if (ImGui::TreeNode(String::ToString(object->Name()).c_str()))
		{
			if (ImGui::TreeNode("Bone Datas"))
			{
				if (model->BoneByIndex(1)->ParentIndex() == -1)
					GetBoneName(model->BoneByIndex(1));
				else
					GetBoneName(model->BoneByIndex(0));

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Mesh Datas"))
			{
				GetMeshData();

				ImGui::TreePop();
			}
			else
				materialIndex = -1;

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void ModelTool::RenderInspector()
{
	ImGui::Begin("Inspector", &MyGui::show_app_inspector, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	{
	}
	ImGui::End();
}

void ModelTool::RenderAnimation()
{
	ImGui::Begin("Animation", &MyGui::show_app_animation, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	{
	}
	ImGui::End();
}

void ModelTool::GetBoneName(ModelBone * bone)
{
	wstring temp = bone->Name();
	string boneName = String::ToString(temp);

	ImGuiTreeNodeFlags nodeFlag = NULL;
	nodeFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	bool b = ImGui::TreeNodeEx(boneName.c_str(), nodeFlag);
	if (b == true)
	{
		boneIndex = bone->Index();
		target = TARGET::BONE;

		for (size_t i = 0; i < bone->ChildCount(); i++)
			GetBoneName(bone->GetChilds()[i]);

		ImGui::TreePop();
	}
}

void ModelTool::GetMeshData()
{
	int meshCount = model->MeshCount();

	for (int i = 0; i < meshCount; i++)
	{
		ModelMesh * mesh = model->Meshes()[i];
		string meshName = String::ToString(mesh->Name());

		if (ImGui::TreeNode(meshName.c_str()))
		{
			target = TARGET::MESH;
			boneIndex = -1;

			meshIndex = i;
			GetMaterialData();

			ImGui::TreePop();
		}
	}
}

void ModelTool::GetMaterialData()
{
	ModelMesh * mesh = model->Meshes()[meshIndex];

	int meshPartsCount = mesh->MeshParts().size();

	for (int i = 0; i < meshPartsCount; i++)
	{
		ModelMeshPart * meshParts = mesh->MeshParts()[i];

		//	Diffuse Map
		if (meshParts->GetMaterial()->GetDiffuseMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetDiffuseMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
		//	Normal Map
		if (meshParts->GetMaterial()->GetNormalMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetNormalMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
		//	Specular Map
		if (meshParts->GetMaterial()->GetSpecularMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetSpecularMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
	}
}