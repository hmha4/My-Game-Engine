#pragma once
#include "Systems/IExecute.h"
#include "Module/LightningRenderer.h"

class TestLightning : public IExecute
{
public:
	enum class Type : UINT
	{
		EConnector = 0,
		EFence = 1,
		ECoil = 2,
		EChain = 3,
		Num = 4,
	};
public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;
	void ImGuiRender();

	void CreateThunder(vector<pair<D3DXVECTOR3, D3DXVECTOR3>>& posSeeds, Type type, int rep, float jit = 1.0f);

	struct ArenaSettings
	{
		ArenaSettings() :
			Fence(false),
			InterCoil(false),
			CoilHelix(false),
			Chain(false),
			Scene(true),
			Glow(true),
			Lines(false),
			AnimationSpeed(15)
		{
			BlurSigma = D3DXVECTOR3(0, 0, 0);
		}

		bool	Fence;
		bool	InterCoil;
		bool	CoilHelix;
		bool	Chain;
		bool	Scene;
		bool	Lines;

		float   AnimationSpeed;
		bool	Glow;
		D3DXVECTOR3	BlurSigma;

		LightningAppearance Beam;

	} Settings;

private:
	struct SeedRecord
	{
		SeedRecord() :
			Closed(false)
		{
		}
		string Name;

		vector<D3DXVECTOR3> Vertices;
		vector<D3DXVECTOR3> InterpolatedVertices;

		bool Closed;
	};
private:
	void ReadSeed(wstring fileName);
	void CreateLightning();
	
	map<string, SeedRecord > seeds;

	class LightningRenderer	*	m_lightning_renderer;

	class PathLightning*		m_inter_coil_lightning;
	class PathLightning*		m_fence_lightning;
	class PathLightning*		m_coil_helix_lightning;
	class ChainLightning*		m_chain_lightning;

	LightningAppearance	m_red_beam;
	LightningAppearance	m_blue_beam;
	LightningAppearance	m_blue_cyan_beam;

	LightningStructure structure[4];

	float m_time;
	bool do_lightning;

	class GizmoGrid * grid;
};