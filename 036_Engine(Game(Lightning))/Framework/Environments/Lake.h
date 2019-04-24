#pragma once
#include "Interfaces/IAlpha.h"

class Lake : public IAlpha
{
public:
	Lake(class ScatterSky * sky, class Terrain * terrain);
	~Lake();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render() override;

	void ImGuiRender();

private:
	void CreateVertices();
	void UpdateReflectionCam();
	void UpdateReflectionMap();
	void UpdateRefractionMap();

	void InitEffectVariables();
	void SetFirstEffectVariables();
private:
	Effect * effect;

	RenderTarget * refractionRTV;
	RenderTarget * reflectionRTV;

	Texture * waterBumpMap;

	ID3D11Buffer * vertexBuffer;

	D3DXMATRIX reflectionView;
	D3DXMATRIX world;
private:
	float waterLevel;
	float waveHeight;
	float waterSpeed;

	float specularLightPowerValue;			//	exponent
	float specularLightPerturbationValue;	//	displacement power

	int fresnelMode;
	int fresnelCount;

	float reflectionRefractionRatio;

	float xdullBlendFactor;

	bool renderElements[2];
private:
	class ScatterSky * sky;
	class Terrain * terrain;

private:
	//	Parameters for rendering
	ID3DX11EffectMatrixVariable * worldVar;
	ID3DX11EffectMatrixVariable * reflectionViewVar;
	ID3DX11EffectScalarVariable * drawModeVar;
	ID3DX11EffectScalarVariable * fresnelModeVar;
	ID3DX11EffectScalarVariable * xdullBlendFactorVar;

	//	Parameters for the wave
	ID3DX11EffectScalarVariable * waterSpeedVar;
	ID3DX11EffectScalarVariable * waveLengthVar;
	ID3DX11EffectScalarVariable * waveHeightVar;
	ID3DX11EffectScalarVariable * windForceVar;
	ID3DX11EffectMatrixVariable * windDirectionVar;
	ID3DX11EffectScalarVariable * specPowerVar;
	ID3DX11EffectScalarVariable * specPerturbationVar;
};