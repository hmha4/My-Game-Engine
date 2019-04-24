#pragma once

class AnimInstance
{
public:
	AnimInstance(class Model * model, wstring shaderFile);
	AnimInstance(class Model * model, Effect*effect);
	~AnimInstance();

	void Ready();
	void Update();
	void Render(UINT tech = 2);

	void AddClip(class ModelClip*clip);
	class Model*GetModel();
public:
	void AddWorld(class GameAnimator*anim);
	void DeleteWorld(UINT instanceID);

	void UpdateWorld(UINT instanceNumber, D3DXMATRIX&world);
	void StartClip(UINT instanceNumber);
	void InitClip(UINT instanceNumber);

	void UpdateAnimation(UINT instanceNumber, UINT curFrame, UINT nextFrame, float frameTime, float Time);
	void UpdateBlending(UINT instanceNumber, UINT curFrame, UINT nextFrame, float blendTime, UINT nextClipIdx);

private:
	class Model * model;

	vector<class GameAnimator*> anims;
	vector<wstring> clipNames; // 중복방지하려고

	UINT maxCount;

	vector<ID3D11Texture2D *> transTextures;
	TextureArray*textureArray;

	D3DXMATRIX worlds[64];

	struct FrameDesc
	{
		UINT CurFrame;
		UINT NextFrame;
		float Time;
		float FrameTime;
		float BlendTime;
		UINT playClipIdx;
		UINT nextClipIdx;
		float padding;
	};
	FrameDesc frames[64];

private:
	vector<ID3DX11EffectVariable*> frameDescVars;
	UINT maxFrameCount = 0;

private:
	vector<ID3DX11EffectVectorVariable*> frustumVars;
	class Frustum*frustum;
	D3DXPLANE plane[6];
};