#pragma once
#include "Objects/IGameObject.h"

class ScatterSky : public IGameObject
{
public:
	ScatterSky();
	~ScatterSky();

	void Initialize();
	void Ready();

	void Update();
	void PreRender();
	void Render();
	void RayleighMieRender();
	void PostRender();
	void ResizeScreen();

	D3DXCOLOR GetSunColor() { return sunColor; }

	void CalcWaveLength();
private:
	void GenerateSphere();
	void GenerateQuad();

	float GetStarIntensity();

	D3DXVECTOR3 GetDirection();

	D3DXCOLOR GetSunColor(float fTheta, int nTurbidity);

private:
	bool realTime;

	wstring skyName;
	wstring tag;

	float theta, phi, setAngle;
	float prevTheta, prevPhi;

	Shader* shader;
	RenderTarget* mieTarget, *rayleighTarget;

	UINT radius, slices, stacks;

	VertexTexture* quadVertices;
	ID3D11Buffer* quadBuffer;

	Render2D* rayleigh2D;
	Render2D* mie2D;
	Render2D* noise2D;

	D3DXMATRIX world;

	Texture* starField;
	Texture* moon;
	Texture* moonGlow;
	Texture* cloud;

private:
	wstring loadStarMap;
	wstring loadMoonMap;
	wstring loadGlowMap;
	wstring loadcloudMap;

public:
	void SetTextures(wstring fileName, UINT type);


public:
	ID3D11ShaderResourceView * MieSRV() { return mieTarget->SRV(); }
	ID3D11ShaderResourceView * RayleighSRV() { return rayleighTarget->SRV(); }

	Texture * &GetStarFieldMap() { return starField; }
	Texture*&GetMoonMap() { return moon; }
	Texture*&GetGlowMap() { return moonGlow; }
	Texture*&GetCloudMap() { return cloud; }

	float&Theta() { return theta; }
	float&Leans() { return setAngle; }
	float&TimeFactor() { return timeFactor; }
	float&StartTheta() { return startTheta; }
	Shader*GetShader() { return shader; }
	D3DXVECTOR3&GetWaveLength() { return WaveLength; }
	int&GetSampleCount() { return SampleCount; }
	bool&IsShowLM() { return isShowLM; }

private:
	float timeFactor;
	float startTheta;
	D3DXVECTOR3 lightDirection;

	bool isPick;

private:
	float MoonAlpha;
	bool isShowLM;

	D3DXVECTOR3 WaveLength;
	int SampleCount;
	D3DXVECTOR3 InvWaveLength;
	D3DXVECTOR3 WaveLengthMie;
	D3DXCOLOR sunColor;

	UINT vertexCount, indexCount;
	ID3D11Buffer *vertexBuffer, *indexBuffer;


public:
	// IGameObject을(를) 통해 상속됨
	bool IsPicked() { return isPick; }
	void SetPickState(bool val) { isPick = val; }

	void Position(float x, float y, float z) override {}
	void Position(D3DXVECTOR3& vec) override {}
	void Position(D3DXVECTOR3* vec) override {}

	void Rotation(float x, float y, float z) override {}
	void Rotation(D3DXVECTOR3& vec) override {}
	void Rotation(D3DXVECTOR3* vec) override {}

	void RotationDegree(float x, float y, float z) override {}
	void RotationDegree(D3DXVECTOR3& vec) override {}
	void RotationDegree(D3DXVECTOR3* vec) override {}

	void Scale(float x, float y, float z) override {}
	void Scale(D3DXVECTOR3& vec) override {}
	void Scale(D3DXVECTOR3* vec) override {}

	void Matrix(D3DXMATRIX* mat) override {}

	void Forward(D3DXVECTOR3* vec) override {}
	void Up(D3DXVECTOR3* vec) override {}
	void Right(D3DXVECTOR3* vec) override {}

	wstring & Name() override;
	wstring & Tag() override;
	void Save() override;
	void Load(wstring fileName) override;
	void Delete() override;
};