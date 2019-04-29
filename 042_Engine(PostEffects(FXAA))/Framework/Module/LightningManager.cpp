#include "FrameWork.h"
#include "LightningManager.h"
#include "Particles/LightningSeed.h"

LightningManager::LightningManager()
{
	Initialize();
	Ready();

	isRunning = 0;
	targets.resize(8, D3DXVECTOR4(0, 0, 0, 0));
}

LightningManager::~LightningManager()
{
	for (pair<wstring, LightningSeed *> lightning : lightnings)
	{
		m_lightning_renderer->DestroyLightning(lightning.second);
	}
	lightnings.clear();
	SAFE_DELETE(m_lightning_renderer);
}

void LightningManager::Initialize()
{
	CreateLightning();

	Glow = false;
	BlurSigma = D3DXVECTOR3(1, 0, 0);
}

void LightningManager::Ready()
{
	m_lightning_renderer = new LightningRenderer();
	m_lightning_renderer->Initialize();
	m_lightning_renderer->Ready();
}


void LightningManager::Update()
{

}

void LightningManager::PreRender()
{
	if (isRunning <= 0) return;

	m_lightning_renderer->PreRender();
}

void LightningManager::Render()
{
	//if (isRunning <= 0) return;

	for (pair<wstring, LightningSeed *> lightning : lightnings)
	{
		if (lightning.first.find(L"Chain") != wstring::npos)
		{
			ChainLightning * chain = dynamic_cast<ChainLightning*>(lightning.second);
			if (chain != NULL)
			{
				chain->Properties.ChainSource = startPos;
				chain->Properties.NumTargets = targets.size();

				for (int i = 0; i < chain->Properties.NumTargets; ++i)
				{
					if (targets[i].w == 0) continue;
					chain->Properties.ChainTargetPositions[i] = targets[i];
				}
			}
			
			m_lightning_renderer->Render(chain);
		}
		else
		{
			m_lightning_renderer->Render(lightning.second);
		}
	}
}

void LightningManager::PostRender()
{
	if (isRunning <= 0) return;
	m_lightning_renderer->PostRender(Glow, BlurSigma);
}

void LightningManager::ResizeScreen()
{
}



void LightningManager::CreateLightning(wstring name, LightSettings& setting, vector<pair<D3DXVECTOR3, D3DXVECTOR3>>& posSeeds, Type type, int rep, float jit)
{
	D3DXVECTOR3 up(0, 1, 0);
	vector<LightningPathSegment> seeds;

	int replicates = rep;
	float jitter = jit;

	for (int j = 0; j < replicates; ++j)
	{
		for (pair<D3DXVECTOR3, D3DXVECTOR3> seed : posSeeds)
		{
			seeds.push_back
			(
				LightningPathSegment
				(
					seed.first,
					seed.second,
					up
				)
			);
		}
	}

	for (size_t i = 0; i < seeds.size(); ++i)
	{
		seeds.at(i).Start += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
		seeds.at(i).End += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
	}

	if (type == LightningManager::Type::EChain)
		lightnings[name] = m_lightning_renderer->CreateChainLightning(0x08, 5);
	else
		lightnings[name] = m_lightning_renderer->CreatePathLightning(seeds, 0x08, 5);

	lightnings[name]->Structure = structure[(UINT)type];
	lightnings[name]->Settings = setting;
	lightnings[name]->LocalWidth(setting.Beam.BoltWidth.x);
	lightnings[name]->Settings.Beam.BoltWidth.x = 0.0f;
}

void LightningManager::Empty()
{
	for (pair<wstring, LightningSeed *> lightning : lightnings)
	{
		m_lightning_renderer->DestroyLightning(lightning.second);
	}
	lightnings.clear();
}

void LightningManager::InputTarget(UINT id, D3DXVECTOR4 target)
{
	targets[id] = target;
}

void LightningManager::RemoveTarget(UINT id)
{
	targets[id] = D3DXVECTOR4(0, 0, 0, 0);
}

void LightningManager::InputStartPos(D3DXVECTOR3 start)
{
	startPos = start;
}

void LightningManager::Start(wstring name, D3DXMATRIX t)
{
	if (lightnings[name]->Settings.InUse == true) return;

	lightnings[name]->Settings.InUse = true;
	lightnings[name]->World(t);

	isRunning++;
}

void LightningManager::End(wstring name)
{
	if (lightnings[name]->Settings.InUse == false) return;

	lightnings[name]->Settings.InUse = false;

	isRunning--;
}

void LightningManager::CreateLightning()
{
	//	Connector
	{
		structure[(UINT)Type::EConnector].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EConnector].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EConnector].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EConnector].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EConnector].ForkLengthDecay = 0.01f;
	}

	//	Fence
	{
		structure[(UINT)Type::EFence].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EFence].ZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationForward = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EFence].ForkLengthDecay = 0.01f;
	}

	//	Coil
	{
		structure[(UINT)Type::ECoil].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::ECoil].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::ECoil].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::ECoil].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::ECoil].ForkLengthDecay = 0.01f;
	}

	//	Chain
	{
		structure[(UINT)Type::EChain].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EChain].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EChain].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EChain].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EChain].ForkLengthDecay = 0.01f;
	}

}

LightningManager * LightningManager::instance = NULL;
void LightningManager::Create()
{
	instance = new LightningManager();
}

void LightningManager::Delete()
{
	SAFE_DELETE(instance);
}

LightningManager * LightningManager::Get()
{
	return instance;
}
