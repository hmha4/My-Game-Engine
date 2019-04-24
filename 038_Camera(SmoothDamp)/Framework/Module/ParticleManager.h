#pragma once
#include "Particles\ParticleSystem.h"

class ParticleManager
{
public:
	struct PTinfo
	{
		float velocity;
		float velocityVar;
		int gravity;
		D3DXVECTOR2 size;
		D3DXVECTOR2 startSize;
		D3DXVECTOR2 endSize;
		float perSecontNum;
		float particleCount;

		float lifeTime;
		float startRange;
		float animSpeed;
		vector<wstring> textures;

		ParticleSystem::ParticleType type;
		ParticleSystem::ParticleOption option;
	};
public:
	static void Create();
	static void Delete();
	static ParticleManager*Get() { return instance; }

private:
	static ParticleManager*instance;

public:
	void InputParticle(wstring name, int count, PTinfo ptInfo);

	void Shot(wstring name, D3DXVECTOR3 pos = { 0,0,0 }, D3DXVECTOR3 dir = { 0,0,1 });
	void UnShot(wstring name); // 반복일때만 끄는형식으로

	void Update();
	void Render();

	class ParticleSystem*&GetParticle(wstring name) { return particleMap[name][0]; }
private:
	typedef map < wstring, vector<class ParticleSystem* >> ::iterator mIter;
	typedef map < wstring, int> ::iterator iIter;
private:
	map<wstring, vector<class ParticleSystem*>> particleMap;
	map<wstring, int> particleIdx;
};