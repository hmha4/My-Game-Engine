#pragma once;

//	TODO : FrameFactor ¼öÁ¤

class GameAnimator : public GameRender
{
public:
	enum class Mode
	{
		Play = 0, Pause, Stop,
	};

public:
	GameAnimator(wstring effectFile, wstring matFile, wstring meshFile);
	~GameAnimator();

	void AddClip(wstring clipFile);

	void Play() { this->mode = Mode::Play; }
	void Pause() { this->mode = Mode::Pause; }
	void Stop() { this->mode = Mode::Stop; }

	void Update();
	void Render();

private:
	void UpdateBone();
	void UpdateVertex();
private:
	class Model * model;

	ID3D11Buffer * boneBuffer;
	ID3D11ShaderResourceView * boneSRV;

	Effect * effect;

	Mode mode;

	UINT currentClip;
	int currentKeyframe;
	int nextKeyframe;

	float frameTime;
	float frameFactor;

	D3DXMATRIX *boneTransforms;
	D3DXMATRIX *renderTransforms;

	vector<class ModelClip *> clips;
};