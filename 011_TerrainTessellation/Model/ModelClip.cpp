#include "stdafx.h"
#include "ModelClip.h"
#include "ModelBone.h"
#include "../Utilities/BinaryFile.h"

ModelClip::ModelClip(wstring file, float startTime)
	: playTime(startTime)
	, bLockRoot(false), bRepeat(false), speed(1.0f)
	, currFrame(0), isPlay(false)
{
	BinaryReader* r = new BinaryReader();
	r->Open(file);

	name = String::ToWString(r->String());
	duration = r->Float();
	frameRate = r->Float();
	frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe* keyframe = new ModelKeyframe();
		keyframe->boneName = String::ToWString(r->String());

		keyframe->index = r->UInt();
		keyframe->parentIndex = r->UInt();

		keyframe->duration = duration;
		keyframe->frameCount = frameCount;
		keyframe->frameRate = frameRate;

		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->transforms.assign(size, ModelKeyframe::Transform());

			void* ptr = (void *)&keyframe->transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframe::Transform) * size);
		}

		keyframeMap.insert(Pair(keyframe->boneName, keyframe));
	}

	BindingKeyframe();

	r->Close();
	SAFE_DELETE(r);
}

ModelClip::~ModelClip()
{
	for (Pair keyframe : keyframeMap)
		SAFE_DELETE(keyframe.second);
}

void ModelClip::Reset()
{
	bRepeat = false;
	speed = 0.0f;
	playTime = 0.0f;
	currFrame = false;
	isPlay = false;
}




void ModelClip::PostRender()
{
	ImGui::Begin("Animation", &MyGui::show_app_animation, ImGuiWindowFlags_MenuBar);
	{
		int tempSpeed = (int)speed;
		ImGui::PushItemWidth(75.0f);
		float i = ImGui::GetWindowContentRegionWidth();
		ImGui::SameLine(i - 120.0f);
		ImGui::Text("Speed");
		ImGui::SameLine();
		ImGui::InputInt(" ", &tempSpeed);
		ImGui::PushItemWidth(i - 80.0f);
		speed = (float)tempSpeed;

		ImGui::Spacing(); ImGui::Spacing();
		ImGui::Text("Frames");
		ImGui::Spacing(); ImGui::SameLine(50);
		ImGui::SliderInt("", &currFrame, 0, frameCount - 1);
	}
	ImGui::End();
}



void ModelClip::BindingKeyframe()
{
	size_t i = (GetKeyframeByIndex(0) == NULL) ? 1 : 0;

	for (i; i < keyframeMap.size(); i++)
	{
		ModelKeyframe * keyframe = GetKeyframeByIndex(i);

		if (keyframe->parentIndex > -1)
		{
			keyframe->parent = GetKeyframeByIndex(keyframe->parentIndex);
			keyframe->parent->childs.push_back(keyframe);
		}
		else
		{
			keyframe->parent = NULL;
		}
	}
}

void ModelClip::DeleteRetargetData()
{
	for (Pair temp : retargetFrame)
	{
		SAFE_DELETE(temp.second);
	}

	retargetFrame.clear();
}

void ModelClip::ShowKeyframeData()
{
	ModelKeyframe * keyframe = NULL;

	if(GetKeyframeByIndex(0) == NULL)
		keyframe = GetKeyframeByIndex(1);
	else
		keyframe = GetKeyframeByIndex(0);

	GetKeyframeData(keyframe);
}

void ModelClip::GetKeyframeData(ModelKeyframe * keyframe)
{
	string boneName = String::ToString(keyframe->boneName);

	if (ImGui::TreeNode(boneName.c_str()))
	{
		for (size_t i = 0; i < keyframe->childs.size(); i++)
			GetKeyframeData(keyframe->childs[i]);

		ImGui::TreePop();
	}
}

ModelKeyframe * ModelClip::GetKeyframeByIndex(UINT index)
{
	uMapKeyframe::iterator it = keyframeMap.begin();
	ModelKeyframe * keyframe = NULL;

	for (it; it != keyframeMap.end(); it++)
	{
		if ((*it).second->index == index)
		{
			keyframe = (*it).second;
			break;
		}
	}

	return keyframe;
}

D3DXMATRIX ModelClip::GetKeyframeMatrix(ModelBone * bone, float time)
{
	wstring boneName = bone->Name();

	uMapKeyframe::iterator it = keyframeMap.find(boneName);
	if (it == keyframeMap.end())
	{
		D3DXMATRIX temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}

	ModelKeyframe *keyframe = keyframeMap.at(boneName);

	playTime += speed * time;
	if (bRepeat == true)
	{
		if (playTime >= duration)
		{
			while (playTime - duration >= 0)
				playTime -= duration;
		}
	}
	else
	{
		if (playTime >= duration)
		{
			playTime = duration;
			isPlay = false;
		}
	}

	D3DXMATRIX invGlobal = bone->Global();
	//	글로벌의 역행렬 : 자기 본의 로컬
	D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

	//	애니메이션이 된 매트릭스
	D3DXMATRIX animation = keyframe->GetInterpolatedMatrix(playTime, bRepeat, currFrame);

	D3DXMATRIX parent;
	int parentIndex = bone->ParentIndex();
	if (parentIndex < 0)
	{
		if (bLockRoot == true)
			D3DXMatrixIdentity(&parent);
		else
			parent = animation;
	}
	else
	{
		parent = animation * bone->Parent()->Global();
	}

	return  invGlobal * parent;
}

void ModelClip::UpdateKeyframe(ModelBone * bone, float time)
{
	D3DXMATRIX animation = GetKeyframeMatrix(bone, time);

	bone->Local(animation);
}
