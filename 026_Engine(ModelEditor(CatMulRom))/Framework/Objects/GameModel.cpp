#include "Framework.h"
#include "GameModel.h"
#include "Models/Model.h"
#include "Models/ModelMesh.h"
#include "Models/ModelMeshPart.h"
#include "Module/TrailRenderer.h"

GameModel::GameModel(wstring effectFile, wstring matFile, wstring meshFile)
	: effectFile(effectFile), matFile(matFile), meshFile(meshFile)
	, isChild(false), parentName(L""), parentBone(NULL)
	, pickCollider(NULL), pass(0)
	, hasBT(false), btName(L"")
{
	effect = new Effect(Effects + L"009_ComputeModel.fx", true);
	boneIndexVariable = effect->AsScalar("BoneIndex");
	boneBufferVariable = effect->AsSRV("BoneBuffer");
	vertexVariable = effect->AsSRV("Vertex");
	resultVariable = effect->AsUAV("Result");

	D3DXMatrixIdentity(&parentBoneWorld);

	//pickCollider = new DebugLine();
	//pickCollider->DrawBoundingBox(model->Min(), model->Max());
	//pickCollider->Name(L"Pick Collider");
	//colliderBoxes.push_back(pickCollider);

	//rimBuffer = new RimBuffer();
}

GameModel::~GameModel()
{
	for (ID3D11Buffer * buffer : vertexBuffers)
		SAFE_RELEASE(buffer);
	for (ID3D11ShaderResourceView * buffer : vertexSRVs)
		SAFE_RELEASE(buffer);
	for (ID3D11UnorderedAccessView * buffer : vertexUAVs)
		SAFE_RELEASE(buffer);
	for (ColliderElement * col : colliders)
		SAFE_DELETE(col);
	for (TrailRenderer * trial : trails)
		SAFE_DELETE(trial);

	SAFE_RELEASE(boneBuffer);
	SAFE_RELEASE(boneSRV);

	//	Delete Bone Transforms
	{
		SAFE_DELETE_ARRAY(boneTransforms);
		SAFE_DELETE_ARRAY(renderTransforms);
	}

	SAFE_DELETE(pickCollider);
	SAFE_DELETE(model);
	SAFE_DELETE(effect);
}

void GameModel::Ready()
{
	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);

	SetEffect(effectFile);

	for (ModelMesh*mesh : model->Meshes())
	{
		ID3D11Buffer*vertexBuffer;
		ID3D11ShaderResourceView*vertexSrv;
		ID3D11UnorderedAccessView*vertexUav;

		CsResource::CreateRawBuffer
		(
			sizeof(VertexTextureNormalTangentBlend) * mesh->VertexCount(),
			mesh->Vertices(),
			&vertexBuffer, false, false
		);

		CsResource::CreateSRV(vertexBuffer, &vertexSrv);
		CsResource::CreateUAV(mesh->VertexBuffer(), &vertexUav);

		vertexBuffers.push_back(vertexBuffer);
		vertexSRVs.push_back(vertexSrv);
		vertexUAVs.push_back(vertexUav);
	}

	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	CsResource::CreateStructuredBuffer(sizeof(D3DXMATRIX), model->BoneCount(), NULL, &boneBuffer, true, false);
	CsResource::CreateSRV(boneBuffer, &boneSRV);

	//pickCollider = new BBox();
	//pickCollider->Min = model->Min();
	//pickCollider->Max = model->Max();

	UpdateWorld();

	model->CreateMinMax(Transformed());
	model->UpdateMinMax(Transformed());
}

void GameModel::Update()
{
	for (TrailRenderer * trail : trails)
		trail->Update();

	if (IsChild() == true)
	{
		parentBoneWorld = ParentBone()->World();

		D3DXVECTOR3 scale, trans, rot;
		D3DXQUATERNION quat;
		D3DXMatrixDecompose(&scale, &quat, &trans, &parentBoneWorld);
		Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

		Rotation(rot);
		Position(trans);
	}
}

void GameModel::UpdateWorld()
{
	__super::UpdateWorld();

	for (Material* material : model->Materials())
		material->GetWorldVariable()->SetMatrix(Transformed());

	UpdateTransforms();
	MappedBoneBuffer();
	UpdateVertex();

	//	Collider
	//D3DXVECTOR3 position = Position();
	//D3DXVECTOR3 scale = Scale();
	//D3DXMATRIX S, T;
	//
	//D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	//D3DXMatrixTranslation(&T, position.x, position.y, position.z);
	//
	//model->UpdateMinMax(S*T);
	//
	//pickCollider->Min = model->Min();
	//pickCollider->Max = model->Max();

	//LineUpdate(L"Pick Collider");
}

void GameModel::UpdateVertex()
{
	for (size_t i = 0; i < model->Meshes().size(); i++)
	{
		ModelMesh * mesh = model->Meshes()[i];

		boneIndexVariable->SetInt(mesh->ParentBoneIndex());
		vertexVariable->SetResource(vertexSRVs[i]);
		resultVariable->SetUnorderedAccessView(vertexUAVs[i]);

		effect->Dispatch(0, pass, (UINT)ceilf(mesh->VertexCount() / 512.0f), 1, 1);
	}
}

void GameModel::UpdateTransforms()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);

		D3DXMATRIX parentTransform;
		D3DXMATRIX global = bone->Global();

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0)
			D3DXMatrixIdentity(&parentTransform);
		else
			parentTransform = boneTransforms[parentIndex];

		boneTransforms[i] = bone->Root() * parentTransform;
		renderTransforms[i] = global * boneTransforms[i];

		D3DXMATRIX S;
		if (pass == 1)
			D3DXMatrixScaling(&S, 0.01f, 0.01f, 0.01f);
		else
			D3DXMatrixIdentity(&S);

		bone->World(renderTransforms[i] * S * Transformed());
	}
}

void GameModel::MappedBoneBuffer()
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, renderTransforms, sizeof(D3DXMATRIX) * model->BoneCount());
	}
	D3D::GetDC()->Unmap(boneBuffer, 0);

	boneBufferVariable->SetResource(boneSRV);
}

void GameModel::Render()
{
	for (ModelMesh* mesh : model->Meshes())
		mesh->Render();

	for (TrailRenderer * trail : trails)
		trail->Render();
}

void GameModel::Clone(void ** clone, wstring fileName)
{
	GameModel * model = new GameModel
	(
		Effects + L"026_Model.fx",
		Models + fileName + L"/" + fileName + L".material",
		Models + fileName + L"/" + fileName + L".mesh"
	);
	model->Ready();
	model->Name() = this->Name();
	model->FileName() = this->FileName();

	*clone = model;
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

bool GameModel::IsPicked()
{
	D3DXVECTOR3 start;
	Context::Get()->GetMainCamera()->Position(&start);

	D3DXMATRIX V, P;
	Context::Get()->GetMainCamera()->MatrixView(&V);
	Context::Get()->GetPerspective()->GetMatrix(&P);

	Ray ray;
	D3DXMATRIX T = Transformed();
	Context::Get()->GetViewport()->GetRay(&ray, start, T, V, P);

	float result;
	bool check = pickCollider->Intersect(&ray, result);

	return check;
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