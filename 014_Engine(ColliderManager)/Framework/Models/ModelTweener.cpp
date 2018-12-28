#include "Framework.h"
#include "ModelTweener.h"
#include "ModelClip.h"
#include "ModelBone.h"

ModelTweener::ModelTweener()
	: current(NULL), next(NULL), blendTime(0.0f), elapsedTime(0.0f)
	, isPaused(false)
{
}

ModelTweener::~ModelTweener()
{
}

bool ModelTweener::IsPlay(ModelClip * clip)
{
	return clip->IsPlay();
}

void ModelTweener::Reset(ModelClip * clip)
{
	clip->Reset();
	current = NULL;
	next = NULL;
	isPaused = false;
}

void ModelTweener::Pause(ModelClip* clip)
{
	if (current != NULL)
		isPaused = !isPaused;
}

void ModelTweener::Play(ModelClip * clip, bool bRepeat, float blendTime, float speed, float startTime)
{
	this->blendTime = blendTime;
	this->elapsedTime = startTime;

	if (current == NULL)
	{
		current = clip;
		current->Reset();
		current->Repeat(bRepeat);
		current->StartTime(startTime);
		current->Speed(speed);

		current->IsPlay() = true;
	}
	else
	{
		if (current != clip)
		{
			next = clip;
			next->Reset();
			next->Repeat(bRepeat);
			next->StartTime(startTime);
			next->Speed(speed);

			next->IsPlay() = true;
		}
	}
}

void ModelTweener::UpdateBlending(ModelBone * bone, float time)
{
	if (current == NULL) return;
	if (isPaused == true) return;

	D3DXMATRIX matrix;
	if (next != NULL)
	{
		elapsedTime += time;
		float t = elapsedTime / blendTime;

		if (t > 1.0f)
		{
			matrix = next->GetKeyframeMatrix(bone, time);

			current = next;
			next = NULL;
		}
		else
		{
			D3DXMATRIX start = current->GetKeyframeMatrix(bone, time);
			D3DXMATRIX end = next->GetKeyframeMatrix(bone, time);

			Math::LerpMatrix(matrix, start, end, t);
		}
	}
	else
	{
		matrix = current->GetKeyframeMatrix(bone, time);
	}

	bone->Local(matrix);
}
