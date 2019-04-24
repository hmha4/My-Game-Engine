#include "Framework.h"
#include "GameModel.h"
#include "Module/TrailRenderer.h"

GameModel::GameModel(UINT instanceID, Effect * effect, wstring matFile, wstring meshFile)
	: GameRender()
	, instanceID(instanceID), matFile(matFile), meshFile(meshFile)
	, effect(effect), instance(NULL)
	, isChild(false), parentName(L""), parentBone(NULL)
	, pickCollider(NULL), pass(0)
	, hasBT(false), btName(L"")
{
	D3DXMatrixIdentity(&parentBoneWorld);
}

GameModel::~GameModel()
{
	for (ColliderElement * col : colliders)
		SAFE_DELETE(col);
	for (TrailRenderer * trial : trails)
		SAFE_DELETE(trial);

	//	Delete Bone Transforms
	{
		SAFE_DELETE_ARRAY(boneTransforms);
		SAFE_DELETE_ARRAY(renderTransforms);
	}

	SAFE_DELETE(pickCollider);

	if(isInstanced == false)
		SAFE_DELETE(model);
}

void GameModel::Ready(bool val)
{
	isInstanced = val;

	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);

	//	모델을 불러오기만 하고 그리진 않을때 isInstanced = false
	if (isInstanced == true)
	{
		instance = InstanceManager::Get()->FindModel(model->GetModelName());
		if (instance != NULL)
		{
			SAFE_DELETE(model);
			model = instance->GetModel();
		}

		InstanceManager::Get()->AddModel(this, Effects + L"032_ModelInstance.fx", InstanceManager::InstanceType::MODEL);

		if (instance == NULL)
			instance = InstanceManager::Get()->FindModel(model->GetModelName());
	}
	
	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	UpdateWorld();

	model->CreateMinMax(Transformed());
	model->UpdateMinMax(Transformed());

	pickCollider = new BBox(model->Min(), model->Max());
}

void GameModel::Update()
{
	for (ColliderElement * collider : colliders)
	{
		int index = collider->BoneIndex();

		if (index != -1)
		{
			D3DXMATRIX boneWorld = boneTransforms[index];
			collider->Transform(boneWorld * Transformed());
		}
		else
			collider->Transform(Transformed());
	}

	for (TrailRenderer * trail : trails)
		trail->Update();

	if (isChild == true)
	{
		parentBoneWorld = ParentBone()->World() * parentModel->Transformed();

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

	InstanceManager::Get()->UpdateWorld(this, instanceID, Transformed(), InstanceManager::InstanceType::MODEL);

	UpdateTransforms();
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

void GameModel::Render()
{

}

void GameModel::RenderShadow(UINT tech, UINT pass)
{
	instance->Render(tech);
}

void GameModel::Clone(void ** clone, wstring fileName)
{
	GameModel * model = new GameModel
	(
		instanceID, effect,
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

void GameModel::SetTrailRun(UINT index, bool val)
{
	trails[index]->Run(val);
}

void GameModel::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	for (Material * material : instance->GetModel()->Materials())
	{
		material->GetEffect()->AsMatrix("LightView")->SetMatrix(v);
		material->GetEffect()->AsMatrix("LightProjection")->SetMatrix(p);
	}
}

void GameModel::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	for (Material * material : instance->GetModel()->Materials())
	{
		material->GetEffect()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);

		if(srv != NULL)
			material->GetEffect()->AsSRV("ShadowMap")->SetResource(srv);
	}
}

void GameModel::ShadowUpdate()
{
	Update();
}

void GameModel::NormalRender()
{
	//Render();
}

void GameModel::ShadowRender(UINT tech, UINT pass)
{
	//RenderShadow(tech, pass);
}

