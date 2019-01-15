#include "Framework.h"
#include "GameAnimator.h"
#include "Models/Model.h"
#include "Models/ModelBone.h"
#include "Models/ModelClip.h"
#include "Models/ModelMesh.h"
#include "Models/ModelMeshPart.h"

GameAnimator::GameAnimator(wstring effectFile, wstring matFile, wstring meshFile)
	: mode(Mode::Stop)
	, currentClip(0), currentKeyframe(0), nextKeyframe(0)
	, frameTime(0.0f), frameFactor(0.0f)
{
	effect = new Effect(Effects + L"009_ComputeModelAnimator.fx", true);

	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);

	for (Material* material : model->Materials())
		material->SetEffect(effectFile);

	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	CsResource::CreateStructuredBuffer(sizeof(D3DXMATRIX), model->BoneCount(), NULL, &boneBuffer);
	CsResource::CreateSRV(boneBuffer, &boneSRV);
}

GameAnimator::~GameAnimator()
{
	SAFE_RELEASE(boneBuffer);
	SAFE_RELEASE(boneSRV);

	//	Delete Bone Transforms
	{
		SAFE_DELETE_ARRAY(boneTransforms);
		SAFE_DELETE_ARRAY(renderTransforms);
	}

	//	Delete ModelClip
	{
		for (ModelClip * clip : clips)
			SAFE_DELETE(clip);
		clips.clear();
		clips.shrink_to_fit();
	}

	SAFE_DELETE(model);
	SAFE_DELETE(effect);
}

void GameAnimator::AddClip(wstring clipFile)
{
	clips.push_back(new ModelClip(clipFile));
}

void GameAnimator::Update()
{
	frameTime += Time::Delta();

	ModelClip * clip = clips[currentClip];
	float invFrameRate = 1.0f / clip->FrameRate();

	if (frameTime > invFrameRate)
	{
		frameTime = 0.0f;

		currentKeyframe = (currentKeyframe + 1) % clip->FrameCount();
		nextKeyframe = (currentKeyframe + 1) % clip->FrameCount();

		UpdateBone();
		UpdateVertex();
	}
}

void GameAnimator::Render()
{
	/*for (Material* material : model->Materials())
	{
	const float* data = renderTransforms[0];
	UINT count = model->BoneCount();

	material->GetEffect()->AsMatrix("Bones")->SetMatrixArray(data, 0, count);
	}*/

	for (ModelMesh* mesh : model->Meshes())
		mesh->Render();
}

void GameAnimator::UpdateBone()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);

		D3DXMATRIX transform;
		D3DXMATRIX parentTransform;

		D3DXMATRIX invGlobal = bone->Global();
		D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

		ModelKeyframe * frame = clips[currentClip]->Keyframe(bone->Name());
		if (frame == NULL) continue;

		D3DXMATRIX S, R, T;

		ModelKeyframeData current = frame->Transforms[currentKeyframe];
		ModelKeyframeData next = frame->Transforms[nextKeyframe];

		D3DXVECTOR3 s1 = current.Scale;
		D3DXVECTOR3 s2 = next.Scale;

		D3DXVECTOR3 s;
		D3DXVec3Lerp(&s, &s1, &s2, frameFactor);
		D3DXMatrixScaling(&S, s.x, s.y, s.z);

		D3DXQUATERNION q1 = current.Rotation;
		D3DXQUATERNION q2 = next.Rotation;

		D3DXQUATERNION q;
		D3DXQuaternionSlerp(&q, &q1, &q2, frameFactor);
		D3DXMatrixRotationQuaternion(&R, &q);

		D3DXVECTOR3 t1 = current.Translation;
		D3DXVECTOR3 t2 = next.Translation;

		D3DXVECTOR3 t;
		D3DXVec3Lerp(&t, &t1, &t2, frameFactor);
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		transform = S * R * T;

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0)
			D3DXMatrixIdentity(&parentTransform);
		else
			parentTransform = boneTransforms[parentIndex];

		boneTransforms[i] = transform * parentTransform;
		renderTransforms[i] = invGlobal * boneTransforms[i];
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, renderTransforms, sizeof(D3DXMATRIX) * model->BoneCount());
	}
	D3D::GetDC()->Unmap(boneBuffer, 0);

	effect->AsShaderResource("BoneBuffer")->SetResource(boneSRV);
}

void GameAnimator::UpdateVertex()
{
	for (ModelMesh * mesh : model->Meshes())
	{
		for (ModelMeshPart * part : mesh->MeshParts())
		{
			vector<VertexTextureNormalTangentBlend> vertices;
			part->Vertices(vertices);

			ID3D11Buffer * vertexBuffer;
			ID3D11ShaderResourceView * vertexSRV;
			ID3D11UnorderedAccessView * vertexUAV;

			CsResource::CreateRawBuffer
			(
				sizeof(VertexTextureNormalTangentBlend) * vertices.size(),
				&vertices[0], &vertexBuffer
			);

			CsResource::CreateSRV(vertexBuffer, &vertexSRV);
			CsResource::CreateUAV(part->VertexBuffer(), &vertexUAV);

			effect->AsShaderResource("Vertex")->SetResource(vertexSRV);
			effect->AsUAV("Result")->SetUnorderedAccessView(vertexUAV);

			effect->Dispatch(0, 0, vertices.size() / 256, 1, 1);

			SAFE_RELEASE(vertexBuffer);
			SAFE_RELEASE(vertexSRV);
			SAFE_RELEASE(vertexUAV);
		}
	}
}