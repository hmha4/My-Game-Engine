#pragma once

//�� �������� ������ ũ��,ȸ��,�̵� ����
struct ModelKeyframeData
{
	float Time;				//�ش�ð�

	D3DXVECTOR3 Scale;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Translation;
};

//�� ��(��)�� ������ ���ϸ��̼� ����
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