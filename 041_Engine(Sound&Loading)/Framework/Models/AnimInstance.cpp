#include "Framework.h"
#include "AnimInstance.h"
#include "Viewer\Frustum.h"

AnimInstance::AnimInstance(Model * model, wstring shaderFile)
	: maxCount(0), model(model)
{
	for (Material*material : model->Materials())
	{
		if (material->GetEffect() == NULL)
		{
			material->SetEffect(shaderFile);
			frustumVars.push_back(material->GetEffect()->AsVector("ClipPlane"));
			frameDescVars.push_back(material->GetEffect()->Variable("Frames"));
		}
	}

	ZeroMemory(frames, sizeof(FrameDesc) * 64);

	
	textureArray = NULL;

	frustum = new Frustum(200);
}

AnimInstance::AnimInstance(Model * model, Effect * effect)
	: maxCount(0), model(model)
{
	for (Material*material : model->Materials())
	{
		if (material->GetEffect() == NULL)
		{
			material->SetEffect(effect);
			frustumVars.push_back(material->GetEffect()->AsVector("ClipPlane"));
			frameDescVars.push_back(material->GetEffect()->Variable("Frames"));
		}
	}


	ZeroMemory(frames, sizeof(FrameDesc) * 64);

	textureArray = NULL;

	frustum = new Frustum(200);
}

AnimInstance::~AnimInstance()
{
	SAFE_DELETE(model);

	for (ID3D11Texture2D*texture : transTextures)
		SAFE_RELEASE(texture);

	SAFE_DELETE(textureArray);
	SAFE_DELETE(frustum);
}

void AnimInstance::Ready()
{
	ID3D11Texture2D *transTexture;
	CsResource::CreateTexture(128 * 4, 400, NULL, &transTexture, true, false);

	D3DXMATRIX **transforms;
	//   UpdateTransform and Map
	{
		transforms = new D3DXMATRIX*[30];
		for (size_t i = 0; i < 30; i++)
			transforms[i] = new D3DXMATRIX[128];

		D3DXMATRIX boneTransforms[128];

		for (UINT c = 0; c < 30; c++)
		{
			for (UINT i = 0; i <model->BoneCount(); i++)
			{
				ModelBone * bone = model->BoneByIndex(i);

				D3DXMATRIX parentTransform;
				D3DXMATRIX global = bone->Global();

				int parentIndex = bone->ParentIndex();
				if (parentIndex < 0)
				{
					//parentTransform = worlds[count];
					D3DXMatrixIdentity(&parentTransform);
				}
				else
					parentTransform = boneTransforms[parentIndex];

				boneTransforms[i] = parentTransform;
				transforms[c][i] = global * boneTransforms[i];
			}
		}


		D3D11_MAPPED_SUBRESOURCE subResource;
		D3D::GetDC()->Map(transTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			for (UINT i = 0; i < 30; i++)
			{
				void * p = ((D3DXMATRIX *)subResource.pData) + (i * 128);

				memcpy(p, transforms[i], sizeof(D3DXMATRIX) * 128);
			}
		}
		D3D::GetDC()->Unmap(transTexture, 0);
	}

	for (UINT i = 0; i < 30; i++)
		SAFE_DELETE_ARRAY(transforms[i]);

	SAFE_DELETE_ARRAY(transforms);

	transTextures.push_back(CsResource::WriteTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, transTexture));

	if (textureArray != NULL)
		SAFE_DELETE(textureArray);

	maxFrameCount = 400;
	textureArray = new TextureArray(transTextures, 128 * 4, maxFrameCount, 1);

	for (Material * material : model->Materials())
		material->GetEffect()->AsSRV("Transforms")->SetResource(textureArray->GetSRV());

}

void AnimInstance::Update()
{
	frustum->GetPlanes(plane);

	for (size_t i = 0; i<frustumVars.size(); i++)
		frustumVars[i]->SetFloatVectorArray((float*)plane, 0, 6);

	for (size_t i = 0; i<frustumVars.size(); i++)
		frameDescVars[i]->SetRawValue(frames, 0, sizeof(FrameDesc) * 64);
}

void AnimInstance::Render(UINT tech)
{
	for (ModelMesh * mesh : model->Meshes())
		mesh->RenderInstance(maxCount, tech);
}

void AnimInstance::AddClip(class ModelClip*clip)
{
	for (size_t i = 0; i < clipNames.size(); i++)
	{
		if (clipNames[i] == clip->AnimName())
			return;
	}

	ID3D11Texture2D *transTexture;
	CsResource::CreateTexture(128 * 4, 400, NULL, &transTexture, true, false);

	D3DXMATRIX **transforms;
	//   UpdateTransform and Map
	{
		transforms = new D3DXMATRIX*[clip->FrameCount()];
		for (size_t i = 0; i < clip->FrameCount(); i++)
			transforms[i] = new D3DXMATRIX[128];

		D3DXMATRIX boneTransforms[128];

		for (UINT c = 0; c < clip->FrameCount(); c++)
		{
			for (UINT b = 0; b < model->BoneCount(); b++)
			{
				ModelBone * bone = model->BoneByIndex(b);

				D3DXMATRIX parentTransform;
				D3DXMATRIX invGlobal = bone->Global();
				D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

				
				D3DXMATRIX animation;
				ModelKeyframe * frame = clip->Keyframe(bone->Name());

				D3DXMATRIX S, R, T1, T2;
				if (frame != NULL)
				{
					ModelKeyframeData data = frame->Transforms[c];

					D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
					D3DXMatrixRotationQuaternion(&R, &data.Rotation);
					D3DXMatrixTranslation(&T1, data.Translation.x, data.Translation.y, data.Translation.z);
					D3DXMatrixTranslation(&T2, data.Translation.x, data.Translation.y, 0);

					//animation = S * R * T;
				}
				//else
				//{
				//	D3DXMatrixIdentity(&animation);
				//}

				int parentIndex = bone->ParentIndex();
				if (parentIndex < 0)
				{
					D3DXMatrixIdentity(&parentTransform);
					parentTransform = S * R * T2;
				}
				else
					parentTransform = S * R * T1 * boneTransforms[parentIndex];


				boneTransforms[b] = parentTransform;
				transforms[c][b] = invGlobal * boneTransforms[b];
			}
		}


		D3D11_MAPPED_SUBRESOURCE subResource;
		D3D::GetDC()->Map(transTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			for (UINT i = 0; i < clip->FrameCount(); i++)
			{
				void * p = ((D3DXMATRIX *)subResource.pData) + (i * 128);

				memcpy(p, transforms[i], sizeof(D3DXMATRIX) * 128);
			}
		}
		D3D::GetDC()->Unmap(transTexture, 0);
	}

	for (UINT i = 0; i < clip->FrameCount(); i++)
		SAFE_DELETE_ARRAY(transforms[i]);

	SAFE_DELETE_ARRAY(transforms);

	transTextures.push_back(CsResource::WriteTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, transTexture));

	if (textureArray != NULL)
		SAFE_DELETE(textureArray);

	if (maxFrameCount <= clip->FrameCount())
		maxFrameCount = clip->FrameCount();
	
	clipNames.push_back(clip->AnimName());
	textureArray = new TextureArray(transTextures, 128 * 4, maxFrameCount, 1);

	for (Material * material : model->Materials())
		material->GetEffect()->AsSRV("Transforms")->SetResource(textureArray->GetSRV());
}

Model * AnimInstance::GetModel()
{
	return model;
}

void AnimInstance::AddWorld(GameAnimator*anim)
{
	anims.push_back(anim);
	worlds[maxCount] = anim->World();

	maxCount++;

	D3D11_MAPPED_SUBRESOURCE subResource;

	for (UINT i = 0; i < model->MeshCount(); i++)
	{
		ID3D11Buffer* instanceBuffer = model->MeshByIndex(i)->InstanceBuffer();

		D3D::GetDC()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			memcpy(subResource.pData, worlds, sizeof(D3DXMATRIX) * maxCount);
		}
		D3D::GetDC()->Unmap(instanceBuffer, 0);
	}
}

void AnimInstance::DeleteWorld(UINT instanceID)
{
	anims.pop_back();
	worlds[instanceID] = 
	{
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0
	};

	D3D11_MAPPED_SUBRESOURCE subResource;
	for (UINT i = 0; i < model->MeshCount(); i++)
	{
		ID3D11Buffer* instanceBuffer = model->MeshByIndex(i)->InstanceBuffer();

		D3D::GetDC()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			memcpy(subResource.pData, worlds, sizeof(D3DXMATRIX) * maxCount);
		}
		D3D::GetDC()->Unmap(instanceBuffer, 0);
	}

	maxCount--;
}

void AnimInstance::UpdateWorld(UINT instanceNumber, D3DXMATRIX & world)
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

void AnimInstance::StartClip(UINT instanceNumber)
{
	frames[instanceNumber].playClipIdx = anims[instanceNumber]->GetPlayClip()->ClipIndex();
}

void AnimInstance::InitClip(UINT instanceNumber)
{
	frames[instanceNumber].playClipIdx = 0;
}

void AnimInstance::UpdateAnimation(UINT instanceNumber, UINT curFrame, UINT nextFrame, float frameTime, float Time)
{
	frames[instanceNumber].CurFrame = curFrame;
	frames[instanceNumber].NextFrame = nextFrame;
	frames[instanceNumber].FrameTime = frameTime;
	frames[instanceNumber].Time = Time;
}

void AnimInstance::UpdateBlending(UINT instanceNumber, UINT curFrame, UINT nextFrame, float blendTime, UINT nextClipIdx)
{
	frames[instanceNumber].CurFrame = curFrame;
	frames[instanceNumber].NextFrame = nextFrame;
	frames[instanceNumber].BlendTime = blendTime;
	frames[instanceNumber].nextClipIdx = nextClipIdx;
}
