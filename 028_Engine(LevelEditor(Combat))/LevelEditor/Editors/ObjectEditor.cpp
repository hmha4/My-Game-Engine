#include "stdafx.h"
#include "ObjectEditor.h"
#include "Module/TrailRenderer.h"

ObjectEditor::ObjectEditor()
	: GameEditor(L"ObjectEditor")
{

}

ObjectEditor::~ObjectEditor()
{
}

void ObjectEditor::Initalize()
{
}

void ObjectEditor::Ready()
{
}

void ObjectEditor::Update()
{
}

void ObjectEditor::PreRender()
{
}

void ObjectEditor::Render()
{
}

void ObjectEditor::HierarchyRender()
{
}

void ObjectEditor::ProjectRender()
{
}

void ObjectEditor::InspectorRender()
{
	if (object == NULL) return;

	ImGui::Text(String::ToString(object->Name()).append(" Inspector").c_str());

	ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 100);
	ImGui::PushItemWidth(70);

	//	Writing gameObject's tag name
	char buf[20];
	strcpy(buf, String::ToString(object->Tag()).c_str());
	if (ImGui::InputText("Tag", buf, 20))
		object->Tag() = String::ToWString(buf);
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	Transform();
	Information();
}

void ObjectEditor::AssetRender()
{
}

void ObjectEditor::Delete()
{
	if (this != NULL)
		delete this;
}

void ObjectEditor::SetGameObjectPtr(IGameObject * gameObject)
{
	object = gameObject;
	model = reinterpret_cast<GameModel*>(object);
}

void ObjectEditor::SetType(UINT type)
{
}

void ObjectEditor::Transform()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		D3DXMATRIX W, V, P;

		object->Matrix(&W);
		Context::Get()->GetMainCamera()->MatrixView(&V);
		Context::Get()->GetPerspective()->GetMatrix(&P);

		MyGui::EditTransform((float*)V, (float*)P, (float*)W);

		D3DXVECTOR3 scale, trans, rot;
		D3DXQUATERNION quat;
		D3DXMatrixDecompose(&scale, &quat, &trans, &W);
		Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

		object->Position(trans);
		object->Rotation(rot);
		object->Scale(scale);
	}
}

void ObjectEditor::Information()
{
	if (ImGui::CollapsingHeader("Information", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool b = false;
		b = ImGui::TreeNodeEx("Colliders", ImGuiTreeNodeFlags_DefaultOpen);
		if (b == true)
		{
			for (ColliderElement * collider : model->GetColliders())
				ImGui::BulletText(String::ToString(collider->Name()).c_str());

			ImGui::TreePop();
		}

		b = ImGui::TreeNodeEx("Trails", ImGuiTreeNodeFlags_DefaultOpen);
		if (b == true)
		{
			ImGui::BulletText("Count : %d", model->GetTrails().size());

			ImGui::TreePop();
		}

		b = ImGui::TreeNodeEx("Behaviour Tree", ImGuiTreeNodeFlags_DefaultOpen);
		if (b == true)
		{
			ImGui::BulletText("Name : %s", String::ToString(model->BTName()).c_str());

			ImGui::TreePop();
		}

		b = ImGui::TreeNodeEx("Child", ImGuiTreeNodeFlags_DefaultOpen);
		if (b == true)
		{
			string str = "false";
			if (model->IsChild() == true) 
				str = "true";

			ImGui::BulletText("Is Child : %s", str.c_str());
			ImGui::BulletText("Parent : %s", model->ParentName().c_str());

			ImGui::TreePop();
		}
	}
}
