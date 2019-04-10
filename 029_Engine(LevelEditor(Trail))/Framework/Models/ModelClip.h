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

	void Reset();
	bool& IsPlay() { return isPlay; }

	void LockRoot(bool val) { bLockRoot = val; }
	bool LockRoot() { return bLockRoot; }
	void Repeat(bool val) { bRepeat = val; }
	bool Repeat() { return bRepeat; }
	void Speed(float val) { speed = val; }
	float Speed() { return speed; }
	void StartTime(float val) { startTime = val; }
	float StartTime() { return startTime; }

	float Duration() { return duration; }
	float FrameRate() { return frameRate; }
	UINT FrameCount() { return frameCount; }

	ModelKeyframe * Keyframe(wstring name);
	ModelKeyframe * Keyframe(UINT index);

	wstring& AnimName() { return clipName; }
	UINT& ClipIndex() 
	{ 
		return clipIndex; 
	}

private:
	wstring clipName;
	wstring name;
	UINT clipIndex;

	float duration;
	float frameRate;
	UINT frameCount;

	bool isPlay;
	bool bLockRoot;
	bool bRepeat;
	float speed;
	float startTime;

	unordered_map<wstring, ModelKeyframe *> keyframeMap;
	typedef pair<wstring, ModelKeyframe *> keyframePair;

	vector<ModelKeyframe*> keyframeVector;
};