#include "Framework.h"
#include "GameModel.h"
#include "../Viewer/Viewport.h"
#include "../Viewer/Freedom.h"
#include "../Viewer/Perspective.h"

GameModel::GameModel(wstring effectFile, wstring matFile, wstring meshFile)
{
	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);

	model->CopyGlobalBoneTo(transforms);

	SetEffect(effectFile);

	pickCollider = new BBox();
	pickCollider->Min = model->Min();
	pickCollider->Max = model->Max();

	//pickCollider = new DebugLine();
	//pickCollider->DrawBoundingBox(model->Min(), model->Max());
	//pickCollider->Name(L"Pick Collider");
	//colliderBoxes.push_back(pickCollider);

	//rimBuffer = new RimBuffer();
}

GameModel::~GameModel()
{
	SAFE_DELETE(pickCollider);
	SAFE_DELETE(model);
}

void GameModel::Update()
{
}

void GameModel::UpdateWorld()
{
	__super::UpdateWorld();

	D3DXMATRIX t;

	if (IsChild() == false)
		t = Transformed();
	else if (IsChild() == true)
	{
		t = ParentBone()->Global() * ParentBone()->World();
		D3DXVECTOR3 ss = Scale();
		D3DXMATRIX S;
		D3DXMatrixScaling(&S, ss.x, ss.y, ss.z);

		t = S * RootAxis() * t;
	}

	model->CopyGlobalBoneTo(transforms, t);

	D3DXVECTOR3 position = Position();
	D3DXVECTOR3 scale = Scale();
	D3DXMATRIX S, T;

	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	model->UpdateMinMax(S*T);

	pickCollider->Min = model->Min();
	pickCollider->Max = model->Max();

	//LineUpdate(L"Pick Collider");
}

void GameModel::Render()
{
	for (Material * material : model->Materials())
	{
		const float* data = transforms[0];
		UINT count = transforms.size();

		material->GetEffect()->AsMatrix("Bones")->SetMatrixArray(data, 0, count);
	}

	for (ModelMesh* mesh : model->Meshes())
	{
		mesh->Render();
	}
}

void GameModel::SetEffect(wstring fileName)
{
	for (Material * material : model->Materials())
		material->SetEffect(fileName);
}

void GameModel::SetEffect(string fileName)
{
	for (Material * material : model->Materials())
		material->SetEffect(fileName);
}

void GameModel::SetDiffuseMap(wstring fileName)
{
	for (Material * material : model->Materials())
		material->SetDiffuseMap(fileName);
}

void GameModel::SetSpecularMap(wstring fileName)
{
	for (Material * material : model->Materials())
		material->SetSpecularMap(fileName);
}

void GameModel::SetNormalMap(wstring fileName)
{
	for (Material * material : model->Materials())
		material->SetNormalMap(fileName);
}

void GameModel::SetDetailMap(wstring fileName)
{
	for (Material * material : model->Materials())
		material->SetDetailMap(fileName);
}

void GameModel::SetDiffuse(float r, float g, float b, float a)
{
	D3DXCOLOR color{ r, g, b, a };

	SetDiffuse(color);
}

void GameModel::SetDiffuse(D3DXCOLOR & color)
{
	for (Material * material : model->Materials())
		material->SetDiffuse(color);
}

void GameModel::SetSpecular(float r, float g, float b, float a)
{
	D3DXCOLOR color{ r, g, b, a };

	SetSpecular(color);
}

void GameModel::SetSpecular(D3DXCOLOR & color)
{
	for (Material * material : model->Materials())
		material->SetSpecular(color);
}

void GameModel::SetShininess(float val)
{
	for (Material * material : model->Materials())
		material->SetShininess(val);
}

//
//DebugLine * GameModel::FindLine(wstring name)
//{
//	for (DebugLine * line : colliderBoxes)
//	{
//		if (line->Name() == name)
//			return line;
//	}
//
//	return NULL;
//}
//
//void GameModel::LineInit(DebugLine *collider)
//{
//	colliderBoxes.push_back(collider);
//}
//
//void GameModel::LineUpdate(wstring name)
//{
//	DebugLine * line = FindLine(name);
//	UINT index = line->BoneIndex();
//
//	if (index == -1)
//	{
//		D3DXVECTOR3 scale = Scale();
//		D3DXMATRIX S;
//		D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
//
//		D3DXMATRIX world = Transformed();
//
//		if(name != L"Pick Collider")
//			world *= S;
//
//		line->Update(world);
//	}
//	else
//	{
//		ModelBone * bone = model->BoneByIndex(index);
//
//		D3DXMATRIX world;
//		world = bone->Global() * bone->World();
//
//		D3DXVECTOR3 scale = Scale();
//		D3DXMATRIX S;
//		D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
//
//		if (name != L"Pick Collider")
//			world *= S;
//
//		line->Update(world);
//	}
//
//
//}
//
//void GameModel::LineRender()
//{
//	for (DebugLine * line : colliderBoxes)
//	{
//		if (line->InUse() == true)
//			line->Render();
//	}
//	//for (pair<wstring, DebugLine*> iter : colliderBoxes)
//	//{
//	//	if(iter.second->InUse() == true)
//	//		iter.second->Render();
//	//}
//}
//
//void GameModel::LineDelete()
//{
//	colliderBoxes.clear();
//}

void GameModel::LoadDiffuseMapFromFile(wstring fileName)
{
	SetDiffuseMap(fileName);
}

void GameModel::LoadSpecularMapFromFile(wstring fileName)
{
	SetSpecularMap(fileName);
}

void GameModel::LoadNormalMapFromFile(wstring fileName)
{
	SetNormalMap(fileName);
}