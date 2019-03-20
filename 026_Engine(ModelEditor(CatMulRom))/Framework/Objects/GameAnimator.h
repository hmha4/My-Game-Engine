#pragma once;

class GameAnimator : public GameModel
{
public:
	enum class Mode
	{
		Play = 0, Pause, Stop,
	};

public:
	GameAnimator(wstring effectFile, wstring matFile, wstring meshFile);
	~GameAnimator();

	void Ready() override;
	void Update() override;
	void ShowFrameData();

	void UpdateBoneWorld();

	void AddClip(wstring clipFile);
	vector<string> &ClipNames() { return clipNames; }
	string &CurrentClipName() { return currentClipName; }

	
	void Play
	(
		string clipName,
		bool bRepeat = false,
		float blendTime = 0.0f,
		float speed = 1.0f,
		float startTime = 0.0f
	);
	void Pause() { this->mode = Mode::Pause; }
	void Stop() {
		this->mode = Mode::Stop;
		InitBoneWorld();
	}


	void Clone(void** clone, wstring fileName) override;

private:
	void UpdateWorld() override;
	void UpdateTransforms() override;

	void InitBoneWorld();

private:
	void AnimationTransform();
	void BlendTransform();

private:
	Mode mode;

	float speed;

	UINT currentClip;
	int currentKeyframe;

	float frameTime;
	float frameFactor;

	float blendTime;
	float elapsedTime;

	vector<string> clipNames;
	string currentClipName;

	map<string, class ModelClip *> clips;

private:
	queue<class ModelClip *> clipQueue;
};