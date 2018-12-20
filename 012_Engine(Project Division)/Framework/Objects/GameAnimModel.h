#pragma once;

class GameAnimModel : public GameModel
{
public:
	GameAnimModel
	(
		wstring matFolder, wstring matFile
		, wstring meshFolder, wstring meshFile
	);
	virtual ~GameAnimModel();

	virtual void Update();

	UINT AddClip(wstring file);
	class ModelClip * GetClip(UINT index) { return clips[index]; }
	int ClipCount() { return clips.size(); }

	void LockRoot(UINT index, bool val);
	void Repeat(UINT index, bool val);
	void Speed(UINT index, float val);

	void Reset(UINT index);
	void Pause(UINT index);
	bool IsPlay(UINT index);
	void Play
	(
		UINT index,
		bool bRepeat = false,
		float blendTime = 0.0f,
		float speed = 1.0f,
		float startTime = 0.0f
	);
	void ShowFrameData(UINT index);

private:
	vector<class ModelClip *> clips;

	class ModelTweener *tweener;
};