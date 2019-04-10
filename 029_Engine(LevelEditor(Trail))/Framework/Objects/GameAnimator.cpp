#include "Framework.h"
#include "GameAnimator.h"

GameAnimator::GameAnimator(UINT instanceID, Effect * effect, wstring matFile, wstring meshFile)
	: GameModel(instanceID, effect, matFile, meshFile)
	, mode(Mode::Stop), instance(NULL)
	, currentClipName("")
	, currentClip(0), currentKeyframe(0), nextKeyframe(0)
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

void GameAnimator::AddClip(UINT clipIndex, wstring clipFile)
{
	wstring fileName = Path::GetFileName(clipFile);
	int k = fileName.find(L".", 0);
	wstring temp(fileName.begin(), fileName.begin() + k);
	fileName = temp;

	clipNames.push_back(String::ToString(fileName));
	clips[String::ToString(fileName)] = new ModelClip(clipFile);
	clips[String::ToString(fileName)]->ClipIndex() = clipIndex;
	clips[String::ToString(fileName)]->AnimName() = fileName;

	instance->AddClip(clips[String::ToString(fileName)]);
}

void GameAnimator::Ready(bool val)
{
	isInstanced = val;

	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);

	isFirstName = model->IsFirstName();

	if (isInstanced == true)
	{
		instance = InstanceManager::Get()->FindAnim(model->GetModelName());
		if (instance != NULL)
		{
			SAFE_DELETE(model);
			model = instance->GetModel();
		}
		InstanceManager::Get()->AddModel(this, Effects + L"033_AnimInstance.fx", InstanceManager::InstanceType::ANIMATION);

		if (instance == NULL)
			instance = InstanceManager::Get()->FindAnim(model->GetModelName());
	}
	
	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	__super::UpdateTransforms();

	model->CreateMinMax(Transformed());
	model->UpdateMinMax(Transformed());

	pickCollider = new BBox(model->Min(), model->Max());
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
		nextKeyframe = 0;

		break;
	}
}

void GameAnimator::RenderShadow(UINT tech, UINT pass)
{
	instance->Render(tech);
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
	this->mode = Mode::Play;

	if (currentClipName == clipName)
	{
		clipQueue.front()->Reset();
		clipQueue.pop();

		ModelClip * clip = clips[clipName];

		this->blendTime = blendTime;
		clip->IsPlay() = true;
		clip->Repeat(bRepeat);
		clip->Speed(speed);
		clip->StartTime(startTime);
		elapsedTime = startTime;
		frameTime = 0.0f;

		currentKeyframe = 0;

		clipQueue.push(clip);

		return;
	}

	ModelClip * clip = clips[clipName];

	this->blendTime = blendTime;
	clip->IsPlay() = true;
	clip->Repeat(bRepeat);
	clip->Speed(speed);
	clip->StartTime(startTime);
	elapsedTime = startTime;
	frameTime = 0.0f;

	if (clipQueue.size() == 0)
	{
		currentClipName = clipName;
		instance->StartClip(instanceID);
	}

	if (clipQueue.size() >= 2)
	{
		clipQueue.front()->Reset();
		clipQueue.pop();
		nextKeyframe = currentKeyframe;
		currentKeyframe = 0;
		clipQueue.push(clip);
	}
	else
	{
		nextKeyframe = currentKeyframe;
		currentKeyframe = 0;
		clipQueue.push(clip);
	}
}

bool GameAnimator::IsPlay()
{
	if (clipQueue.size() > 1)
		return false;

	return clips[currentClipName]->IsPlay();
}

void GameAnimator::Clone(void ** clone, wstring fileName)
{
	GameAnimator * model = new GameAnimator
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

void GameAnimator::UpdateBoneWorld()
{
	if (clips.size() < 1) return;

	UpdateTransforms();
}

void GameAnimator::InitBoneWorld()
{
	__super::UpdateTransforms();
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
			{
				currentKeyframe++;
				nextKeyframe = currentKeyframe + 1;
			}
			else
			{
				currentKeyframe = 0;
				nextKeyframe = 0;
			}

			frameTime = 0.0f;
		}
		else
		{
			if (currentKeyframe < (int)clip->FrameCount() - 2)
			{
				currentKeyframe++;
				nextKeyframe = currentKeyframe + 1;
			}
			else
			{
				currentKeyframe = (int)clip->FrameCount() - 2;
				nextKeyframe = (int)clip->FrameCount() - 2;
				clip->IsPlay() = false;
			}
		}

		UpdateBoneWorld();
	}

	float time = frameTime / invFrameRate;

	instance->UpdateAnimation(instanceID, currentKeyframe, nextKeyframe, frameTime, time);
}

void GameAnimator::BlendTransform()
{
	elapsedTime += Time::Delta();
	float time = elapsedTime / blendTime;

	if (blendTime == 0.0f)
		time = 1.1f;

	if (time <= 1)
	{
		instance->UpdateBlending(instanceID, nextKeyframe, 0, time, clipQueue.back()->ClipIndex());

		UINT boneNum;

		if (isFirstName == false)
			boneNum = 0;
		else
			boneNum = 1;

		for (UINT i = boneNum; i < model->BoneCount(); i++)
		{
			ModelBone*bone = model->BoneByIndex(i);

			int index = bone->Index();
			ModelKeyframe * curClipFrame; 
			ModelKeyframe * nextClipFrame;

			if (isFirstName == false)
			{
				curClipFrame = clipQueue.front()->Keyframe(index);
				nextClipFrame = clipQueue.back()->Keyframe(index);
			}
			else
			{
				curClipFrame = clipQueue.front()->Keyframe(index - 1);
				nextClipFrame = clipQueue.back()->Keyframe(index - 1);
			}


			if (curClipFrame == NULL || nextClipFrame == NULL)
			{
				continue;
			}

			if (nextKeyframe > (int)curClipFrame->Transforms.size() - 1)
				nextKeyframe = (int)curClipFrame->Transforms.size() - 1;

			D3DXMATRIX transform;
			D3DXMATRIX parentTransform;

			D3DXMATRIX invGlobal = bone->Global();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

			D3DXMATRIX S, R, T1, T2;

			ModelKeyframeData current = curClipFrame->Transforms[nextKeyframe];
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
			D3DXMatrixTranslation(&T1, t.x, t.y, t.z);
			D3DXMatrixTranslation(&T2, t.x, t.y, 0);

			//transform = S * R * T;

			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0)
			{
				D3DXMatrixIdentity(&parentTransform);
				parentTransform = S * R * T2;
			}
			else
				parentTransform = S * R * T1 * boneTransforms[parentIndex];

			boneTransforms[i] = parentTransform;
			renderTransforms[i] = invGlobal * boneTransforms[i];

			bone->World(boneTransforms[i]);
		}
	}
	else
	{
		clipQueue.front()->Reset();
		clipQueue.pop();
		elapsedTime = 0.0f;
		currentKeyframe = 0;
		nextKeyframe = 0;

		currentClipName = String::ToString(clipQueue.front()->AnimName());
		instance->StartClip(instanceID);
	}
}

void GameAnimator::UpdateWorld()
{
	GameRender::UpdateWorld();

	InstanceManager::Get()->UpdateWorld(this, instanceID, Transformed(), InstanceManager::InstanceType::ANIMATION);
}

void GameAnimator::UpdateTransforms()
{
	UINT boneNum;

	if (isFirstName == false) 
		boneNum = 0;
	else 
		boneNum = 1;

	for (UINT i = boneNum; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);

		D3DXMATRIX parentTransform;
		D3DXMATRIX invGlobal = bone->Global();
		D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

		D3DXMATRIX transform;
		ModelKeyframe * frame;

		int boneIndex = bone->Index();
		if(isFirstName == false)
			frame = clips[currentClipName]->Keyframe(boneIndex);
		else
			frame = clips[currentClipName]->Keyframe(boneIndex - 1);

		D3DXMATRIX S, R, T1, T2;
		if (frame != NULL)
		{
			ModelKeyframeData current = frame->Transforms[currentKeyframe];

			D3DXVECTOR3 s = current.Scale;
			D3DXMatrixScaling(&S, s.x, s.y, s.z);

			D3DXQUATERNION q = current.Rotation;
			D3DXMatrixRotationQuaternion(&R, &q);

			D3DXVECTOR3 t = current.Translation;
			D3DXMatrixTranslation(&T1, t.x, t.y, t.z);
			D3DXMatrixTranslation(&T2, t.x, t.y, 0);

			//transform = S * R * T;
		}
		else
		{
			D3DXMatrixIdentity(&transform);
		}

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0)
		{
			D3DXMatrixIdentity(&parentTransform);
			parentTransform = S * R * T2;
		}
		else
			parentTransform = S * R * T1 * boneTransforms[parentIndex];
		parentTransform = bone->Root() * parentTransform;

		boneTransforms[i] = parentTransform;
		renderTransforms[i] = invGlobal * boneTransforms[i];

		bone->World(boneTransforms[i]);
	}
}

void GameAnimator::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	for (Material * material : instance->GetModel()->Materials())
	{
		material->GetEffect()->AsMatrix("LightView")->SetMatrix(v);
		material->GetEffect()->AsMatrix("LightProjection")->SetMatrix(p);
	}
}

void GameAnimator::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	for (Material * material : instance->GetModel()->Materials())
	{
		material->GetEffect()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);

		if (srv != NULL)
			material->GetEffect()->AsSRV("ShadowMap")->SetResource(srv);
	}
}

void GameAnimator::ShadowUpdate()
{
	Update();
}

void GameAnimator::ShadowRender(UINT tech, UINT pass)
{
	RenderShadow(tech, pass);
}