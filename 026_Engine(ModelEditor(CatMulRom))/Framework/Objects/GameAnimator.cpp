#include "Framework.h"
#include "GameAnimator.h"
#include "Models/Model.h"
#include "Models/ModelBone.h"
#include "Models/ModelClip.h"
#include "Models/ModelMesh.h"
#include "Models/ModelMeshPart.h"

GameAnimator::GameAnimator(wstring effectFile, wstring matFile, wstring meshFile)
	: GameModel(effectFile, matFile, meshFile)
	, mode(Mode::Stop)
	, currentClipName("")
	, currentClip(0), currentKeyframe(0)
	, frameTime(0.0f), frameFactor(0.0f)
	, blendTime(0.0f), elapsedTime(0.0f)
{
	pass = 1;
}

GameAnimator::~GameAnimator()
{
	for (pair<string, ModelClip *> clip : clips)
		SAFE_DELETE(clip.second);
	clips.clear();
}

void GameAnimator::AddClip(wstring clipFile)
{
	wstring fileName = Path::GetFileName(clipFile);
	int k = fileName.find(L".", 0);
	wstring temp(fileName.begin(), fileName.begin() + k);
	fileName = temp;

	clipNames.push_back(String::ToString(fileName));
	clips[String::ToString(fileName)] = new ModelClip(clipFile);
}

void GameAnimator::Ready()
{
	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);



	SetEffect(effectFile);

	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	CsResource::CreateStructuredBuffer(sizeof(D3DXMATRIX), model->BoneCount(), NULL, &boneBuffer, true, false);
	CsResource::CreateSRV(boneBuffer, &boneSRV);

	__super::UpdateTransforms();
	__super::MappedBoneBuffer();

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

		boneIndexVariable->SetInt(mesh->ParentBoneIndex());
		vertexVariable->SetResource(vertexSrv);
		resultVariable->SetUnorderedAccessView(vertexUav);

		effect->Dispatch(0, 0, (UINT)ceilf(mesh->VertexCount() / 256.f), 1, 1);
	}

	model->CreateMinMax(Transformed());
	model->UpdateMinMax(Transformed());
}

void GameAnimator::Update()
{
	__super::Update();

	if (clips.size() < 1) return;
	if (currentClipName == "") return;

	switch (mode)
	{
	case GameAnimator::Mode::Play:

		if (clipQueue.size() < 2)
			AnimationTransform();
		else
			BlendTransform();

		break;
	case GameAnimator::Mode::Pause:

		break;
	case GameAnimator::Mode::Stop:

		frameTime = 0.0f;
		currentKeyframe = 0;

		break;
	}
}

void GameAnimator::ShowFrameData()
{
	if (clips.size() < 1) return;
	if (currentClipName == "") return;

	int tempSpeed = (int)clips[currentClipName]->Speed();
	ImGui::PushItemWidth(75.0f);
	float i = ImGui::GetWindowContentRegionWidth();
	ImGui::SameLine(i - 120.0f);
	ImGui::Text("Speed");
	ImGui::SameLine();
	if (ImGui::InputInt(" ", &tempSpeed))
		clips[currentClipName]->Speed((float)tempSpeed);
	ImGui::PushItemWidth(i - 80.0f);

	ImGui::Spacing(); ImGui::Spacing();
	ImGui::Text("Frames");
	ImGui::Spacing(); ImGui::SameLine(50);
	ImGui::SliderInt("", &currentKeyframe, 0, clips[currentClipName]->FrameCount() - 2);
}

void GameAnimator::Play(string clipName, bool bRepeat, float blendTime, float speed, float startTime)
{
	if (clipQueue.size() >= 2)
		return;

	this->mode = Mode::Play;

	if (currentClipName == clipName)
		return;

	currentClipName = clipName;
	ModelClip * clip = clips[currentClipName];

	this->blendTime = blendTime;
	clip->IsPlay() = true;
	clip->Repeat(bRepeat);
	clip->Speed(speed);
	clip->StartTime(startTime);
	elapsedTime = startTime;
	frameTime = 0.0f;

	clipQueue.push(clip);
}

void GameAnimator::Clone(void ** clone, wstring fileName)
{
	GameAnimator * model = new GameAnimator
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

void GameAnimator::UpdateBoneWorld()
{
	if (clips.size() < 1) return;

	UpdateTransforms();
	MappedBoneBuffer();
	UpdateVertex();
}

void GameAnimator::InitBoneWorld()
{
	__super::UpdateTransforms();
	__super::MappedBoneBuffer();

	for (size_t i = 0; i < model->Meshes().size(); i++)
	{
		ModelMesh * mesh = model->Meshes()[i];

		boneIndexVariable->SetInt(mesh->ParentBoneIndex());
		vertexVariable->SetResource(vertexSRVs[i]);
		resultVariable->SetUnorderedAccessView(vertexUAVs[i]);

		effect->Dispatch(0, 0, (UINT)ceilf(mesh->VertexCount() / 512.0f), 1, 1);
	}
}

void GameAnimator::AnimationTransform()
{
	ModelClip*clip = clipQueue.front();

	if (clip->Repeat() == false && clip->IsPlay() == false)
		return;

	frameTime += Time::Delta() * clip->Speed();
	float invFrameRate = 1.0f / clip->FrameRate();

	if (frameTime >= invFrameRate)
	{
		frameTime = 0.0f;

		if (clip->Repeat())
		{
			if (currentKeyframe < (int)clip->FrameCount() - 2)
				currentKeyframe++;
			else
				currentKeyframe = 0;

			frameTime = 0.0f;
		}
		else
		{
			if (currentKeyframe < (int)clip->FrameCount() - 2)
				currentKeyframe++;
			else
			{
				currentKeyframe = (int)clip->FrameCount() - 2;
				clip->IsPlay() = false;
			}
		}

		UpdateBoneWorld();
	}
}

void GameAnimator::BlendTransform()
{
	elapsedTime += Time::Delta();
	float time = elapsedTime / blendTime;

	if (time <= 1)
	{
		for (UINT i = 0; i < model->BoneCount(); i++)
		{
			ModelBone*bone = model->BoneByIndex(i);

			UINT index = bone->Index();
			ModelKeyframe * curClipFrame = clipQueue.front()->Keyframe(index);
			ModelKeyframe * nextClipFrame = clipQueue.back()->Keyframe(index);
			if (curClipFrame == NULL || nextClipFrame == NULL)
			{
				continue;
			}

			D3DXMATRIX transform;
			D3DXMATRIX parentTransform;

			D3DXMATRIX invGlobal = bone->Global();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

			D3DXMATRIX S, R, T;

			ModelKeyframeData current = curClipFrame->Transforms[currentKeyframe];
			ModelKeyframeData next = nextClipFrame->Transforms[0];

			D3DXVECTOR3 s1 = current.Scale;
			D3DXVECTOR3 s2 = next.Scale;

			D3DXVECTOR3 s;
			D3DXVec3Lerp(&s, &s1, &s2, time);
			D3DXMatrixScaling(&S, s.x, s.y, s.z);

			D3DXQUATERNION q1 = current.Rotation;
			D3DXQUATERNION q2 = next.Rotation;

			D3DXQUATERNION q;
			D3DXQuaternionSlerp(&q, &q1, &q2, time);
			D3DXMatrixRotationQuaternion(&R, &q);

			D3DXVECTOR3 t1 = current.Translation;
			D3DXVECTOR3 t2 = next.Translation;

			D3DXVECTOR3 t;
			D3DXVec3Lerp(&t, &t1, &t2, time);
			D3DXMatrixTranslation(&T, t.x, t.y, t.z);

			transform = S * R * T;

			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0)
				D3DXMatrixIdentity(&parentTransform);
			else
				parentTransform = boneTransforms[parentIndex];

			boneTransforms[i] = transform * parentTransform;
			renderTransforms[i] = invGlobal * boneTransforms[i];

			bone->World(boneTransforms[i]);
		}

		MappedBoneBuffer();
		UpdateVertex();
	}
	else
	{
		clipQueue.pop();
		elapsedTime = 0.0f;
		currentKeyframe = 0;
	}
}

void GameAnimator::UpdateWorld()
{
	__super::UpdateWorld();

	for (Material* material : model->Materials())
		material->GetWorldVariable()->SetMatrix(Transformed());
}

void GameAnimator::UpdateTransforms()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);

		ModelKeyframe * frame = clips[currentClipName]->Keyframe(bone->Index());
		if (frame == NULL) continue;

		D3DXMATRIX transform;
		D3DXMATRIX parentTransform;

		D3DXMATRIX invGlobal = bone->Global();
		D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

		D3DXMATRIX S, R, T;

		ModelKeyframeData current = frame->Transforms[currentKeyframe];

		D3DXVECTOR3 s = current.Scale;
		D3DXMatrixScaling(&S, s.x, s.y, s.z);

		D3DXQUATERNION q = current.Rotation;
		D3DXMatrixRotationQuaternion(&R, &q);

		D3DXVECTOR3 t = current.Translation;
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		transform = S * R * T;

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0)
			D3DXMatrixIdentity(&parentTransform);
		else
			parentTransform = boneTransforms[parentIndex];
		parentTransform = bone->Root() * parentTransform;

		boneTransforms[i] = transform * parentTransform;
		renderTransforms[i] = invGlobal * boneTransforms[i];

		bone->World(boneTransforms[i]);
	}
}