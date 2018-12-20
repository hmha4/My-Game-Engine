#pragma once
#include "ModelKeyframe.h"

class ModelClip
{
public:
	friend class Model;

public:
	ModelClip(wstring file, float startTime = 0.0f);
	~ModelClip();

	void Reset();
	bool& IsPlay() { return isPlay; }

	void PostRender();

	void BindingKeyframe();
	void DeleteRetargetData();
	void ShowKeyframeData();
	void GetKeyframeData(class ModelKeyframe * keyframe);

	ModelKeyframe * GetKeyframeByIndex(UINT index);
	D3DXMATRIX GetKeyframeMatrix(class ModelBone* bone, float time);
	void UpdateKeyframe(class ModelBone* bone, float time);

	void LockRoot(bool val) { bLockRoot = val; }
	void Repeat(bool val) { bRepeat = val; }
	void Speed(float val) { speed = val; }
	void StartTime(float val) { playTime = val; }

	float Duration() { return duration; }
	float FrameRate() { return frameRate; }
	UINT FrameCount() { return frameCount; }
	bool LockRoot() { return bLockRoot; }
	bool Repeat() { return bRepeat; }
	float Speed() { return speed; }
	float StartTime() { return playTime; }
	
private:
	wstring name;

	float duration;
	float frameRate;
	UINT frameCount;
	int currFrame;

	bool isPlay;
	bool bLockRoot;
	bool bRepeat;
	float speed;
	float playTime;

	typedef pair<wstring, ModelKeyframe *> Pair;
	typedef unordered_map<wstring, ModelKeyframe *> uMapKeyframe;
	uMapKeyframe keyframeMap;
	uMapKeyframe retargetFrame;
};