#pragma once;

class GameAnimator : public GameModel
{
public:
	enum class Mode
	{
		Play = 0, Pause, Stop,
	};

public:
	GameAnimator(UINT instanceID, Effect * effect, wstring matFile, wstring meshFile);
	~GameAnimator();

	void Ready(bool val = true) override;
	void Update() override;
	void RenderShadow(UINT tech = 0, UINT pass = 0) override;
	void ShowFrameData();

	void UpdateBoneWorld();

	// ======================================================================== //
	//	Clips
	// ======================================================================== //
	UINT ClipCount() { return clips.size(); }
	void AddClip(UINT clipIndex, wstring clipFile);

	vector<string> & ClipNames() { return clipNames; }
	string & CurrentClipName() { return currentClipName; }
	ModelClip * GetPlayClip() { return clips[currentClipName]; }
	ModelClip * GetClip(string clipName) { return clips[clipName]; }

	int CurrKeyframe() { return currentKeyframe; }
	void Play(string clipName, bool bRepeat = false, float blendTime = 0.0f, float speed = 1.0f, float startTime = 0.0f);
	void Pause() { this->mode = Mode::Pause; }
	void Stop();
	bool IsPlay();

	// ======================================================================== //
	//	Interfaces
	// ======================================================================== //
	void Clone(void** clone, wstring fileName) override;

private:
	void UpdateWorld() override;
	void UpdateTransforms() override;

	void InitBoneWorld();
	void AnimationTransform();
	void BlendTransform();

private:
	// ======================================================================== //
	//	Interfaces
	// ======================================================================== //
	virtual void SetEffect(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv = NULL) override;
	virtual void ShadowUpdate();
	virtual void ShadowRender(UINT tech, UINT pass) override;
private:
	bool isFirstName;

	Mode mode;

	int currentKeyframe;
	int nextKeyframe;

	float frameTime;	//	Used to calculate animation transform
	float blendTime;	//	Max animation blending time
	float elapsedTime;	//	Elapsed animation blending time

	vector<string> clipNames;	//	Used for ImGui ComboBox
	string currentClipName;		//	Current clip name

	map<string, class ModelClip *> clips;

private:
	queue<class ModelClip *> clipQueue;
	AnimInstance * instance;
};