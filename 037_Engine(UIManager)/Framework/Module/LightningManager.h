#pragma once
#include "Module/LightningRenderer.h"

class LightningManager
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
	LightningManager();
	~LightningManager();

	
	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

	void CreateLightning(wstring name, LightSettings& setting, vector<pair<D3DXVECTOR3, D3DXVECTOR3>>& posSeeds, Type type, int rep, float jit = 1.0f);

	//	Chain¿œ ∂ß∏∏
	void InputTarget(UINT id, D3DXVECTOR4 target);
	void RemoveTarget(UINT id);
	void InputStartPos(D3DXVECTOR3 start);

	void Start(wstring name, D3DXMATRIX t);
	void End(wstring name);

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
	void Initialize();
	void Ready();
	void CreateLightning();

private:
	D3DXVECTOR3 startPos;
	vector<D3DXVECTOR4> targets;
	map<wstring, class LightningSeed *> lightnings;

	class LightningRenderer	*	m_lightning_renderer;

	LightningStructure structure[4];

	bool do_lightning;
	UINT isRunning;

	bool	Glow;
	D3DXVECTOR3	BlurSigma;

public:
	static void Create();
	static void Delete();
	static LightningManager * Get();

	static LightningManager * instance;
};