#pragma once
#include "Interfaces/IAlpha.h"

class Fire : public IAlpha
{
public:
	Fire();
	~Fire();

	void Initialize();
	void Ready(float x, float y, float z);
	void ImGuiRender();
	void Render() override;

	void Setting(bool jitter, int smpRate, float speed, float noiseScale, float shapeSize, float rough);
private:
	void Reset();
private:
	Material * material;
	MeshCube * meshCube;

	bool bJitter;
	int samplingRate;
	
	float speed;
	float noiseScale;
	float roughness;
	float shapeSize;
	float frequencyWeights[5];

	D3DXMATRIX world;
private:
	Texture * fireTexture;

	ID3D11Texture2D * noiseTexture;
	ID3D11ShaderResourceView * jitterSRV;
	ID3D11ShaderResourceView * permSRV;


private:
	ID3DX11EffectMatrixVariable * worldVar;

	ID3DX11EffectScalarVariable * stepSizeVar;
	ID3DX11EffectScalarVariable * noiseScaleVar;
	ID3DX11EffectScalarVariable * roughnessVar;
	ID3DX11EffectScalarVariable * frequencyWeightsVar;
	ID3DX11EffectScalarVariable * speedVar;
	ID3DX11EffectScalarVariable * bJitterVar;

	ID3DX11EffectShaderResourceVariable * screenDepthVar;
	ID3DX11EffectShaderResourceVariable * fireTexVar;
	ID3DX11EffectShaderResourceVariable * jitterTexVar;
	ID3DX11EffectShaderResourceVariable * permTexVar;
};