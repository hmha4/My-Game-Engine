#include "Framework.h"
#include "ModelInstance.h"
#include "Viewer\Frustum.h"

ModelInstance::ModelInstance(Model*model, wstring shaderFile)
	:maxCount(0),model(model)
{
	for (Material*material : model->Materials())
	{
		if (material->GetEffect() == NULL)
		{
			material->SetEffect(shaderFile);
			frustumVars.push_back(material->GetEffect()->AsVector("ClipPlane"));
		}
	}

	frustum = new Frustum(500);
}

ModelInstance::ModelInstance(Model * model, Effect * effect)
	:maxCount(0), model(model)
{
	for (Material*material : model->Materials())
	{
		if (material->GetEffect() == NULL)
		{
			material->SetEffect(effect);
			frustumVars.push_back(material->GetEffect()->AsVector("ClipPlane"));
		}
	}

	frustum = new Frustum(500);
}

ModelInstance::~ModelInstance()
{
	SAFE_DELETE(model);
	SAFE_RELEASE(transTexture);
	SAFE_RELEASE(transSrv);
	SAFE_DELETE(frustum);
}

void ModelInstance::Ready()
{
	//   128개의 매트릭스를 만듬
	//   한 픽셀당 16바이트 이기 때문에 행열은 64바이트 이기 때문에 4를 곱해줌
	CsResource::CreateTexture(128 * 4, 128, NULL, &transTexture, true, false);
	CsResource::CreateSRV(transTexture, &transSrv);

	D3DXMATRIX boneTransforms[128];

	for (UINT i = 0; i < GetModel()->BoneCount(); i++)
	{
		ModelBone * bone = GetModel()->BoneByIndex(i);

		D3DXMATRIX parentTransform;
		D3DXMATRIX global = bone->Global();

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0)
			D3DXMatrixIdentity(&parentTransform);
		else
			parentTransform = boneTransforms[parentIndex];

		boneTransforms[i] = parentTransform;
		transforms[0][i] = global * boneTransforms[i];
	}

	D3D11_MAPPED_SUBRESOURCE subResource;

	HRESULT hr = D3D::GetDC()->Map(transTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	assert(SUCCEEDED(hr));
	{
		for (UINT i = 0; i < 64; i++)
		{
			memcpy((void*)((D3DXMATRIX *)subResource.pData + (i * 128)), transforms[0], sizeof(D3DXMATRIX) * 128);
		}
	}
	D3D::GetDC()->Unmap(transTexture, 0);

	for (Material* material : GetModel()->Materials())
		material->GetEffect()->AsSRV("Transforms")->SetResource(transSrv);
}

void ModelInstance::AddWorld(D3DXMATRIX & world)
{
	worlds[maxCount] = world;
	maxCount++;

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE subResource;
	for (UINT i = 0; i < GetModel()->MeshCount(); i++)
	{
		ID3D11Buffer* instanceBuffer = GetModel()->MeshByIndex(i)->InstanceBuffer();

		hr = D3D::GetDC()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		assert(SUCCEEDED(hr));
		{
			memcpy(subResource.pData, worlds, sizeof(D3DXMATRIX) * maxCount);
		}
		D3D::GetDC()->Unmap(instanceBuffer, 0);
	}
}

void ModelInstance::DeleteWorld(UINT instanceID)
{
	worlds[instanceID] = 
	{
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0
	};

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE subResource;
	for (UINT i = 0; i < GetModel()->MeshCount(); i++)
	{
		ID3D11Buffer* instanceBuffer = GetModel()->MeshByIndex(i)->InstanceBuffer();

		hr = D3D::GetDC()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		assert(SUCCEEDED(hr));
		{
			memcpy(subResource.pData, worlds, sizeof(D3DXMATRIX) * maxCount);
		}
		D3D::GetDC()->Unmap(instanceBuffer, 0);
	}

	maxCount--;
}

Model * ModelInstance::GetModel()
{
	return model;
}

void ModelInstance::UpdateWorld(UINT instanceNumber, D3DXMATRIX&world)
{
	worlds[instanceNumber] = world;

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE subResource;
	for (UINT i = 0; i < GetModel()->MeshCount(); i++)
	{
		ID3D11Buffer* instanceBuffer = GetModel()->MeshByIndex(i)->InstanceBuffer();

		hr = D3D::GetDC()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		assert(SUCCEEDED(hr));
		{
			memcpy(subResource.pData, worlds, sizeof(D3DXMATRIX) * maxCount);
		}
		D3D::GetDC()->Unmap(instanceBuffer, 0);
	}
}

void ModelInstance::Update()
{
	frustum->GetPlanes(plane);

	for(size_t i=0; i<frustumVars.size(); i++)
		frustumVars[i]->SetFloatVectorArray((float*)plane, 0, 6);
}

void ModelInstance::Render(UINT tech)
{
	for (ModelMesh*mesh : GetModel()->Meshes())
		mesh->RenderInstance(maxCount, tech);
}

