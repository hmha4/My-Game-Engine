#pragma once

//한 프레임이 가지는 크기,회전,이동 정보
struct ModelKeyframeData
{
	float Time;				//해당시간

	D3DXVECTOR3 Scale;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Translation;
};

//한 본(뼈)이 가지는 에니메이션 정보
struct ModelKeyframe
{
	wstring BoneName;
	vector<ModelKeyframeData> Transforms;
};

class ModelClip
{
public:
	friend class Model;

public:
	ModelClip(wstring file);
	~ModelClip();

	float Duration() { return duration; }
	float FrameRate() { return frameRate; }
	UINT FrameCount() { return frameCount; }

	ModelKeyframe * Keyframe(wstring name);

private:
	wstring name;

	float duration;
	float frameRate;
	UINT frameCount;

	unordered_map<wstring, ModelKeyframe *> keyframeMap;
	typedef pair<wstring, ModelKeyframe *> keyframePair;
};