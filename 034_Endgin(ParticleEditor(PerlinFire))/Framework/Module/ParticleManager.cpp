#include "Framework.h"
#include "ParticleManager.h"

ParticleManager*ParticleManager::instance = NULL;

void ParticleManager::Create()
{
	if (instance == NULL)
		instance = new ParticleManager();

}

void ParticleManager::Delete()
{
	/*mIter miter = instance->particleMap.begin();

	if (instance->particleMap.size() > 0)
	{
	for (; miter != instance->particleMap.end();)
	{
	for (size_t i = 0; i < miter->second.size(); i++)
	{
	SAFE_DELETE(miter->second[i]);
	}
	}
	}
	*/
	SAFE_DELETE(instance);
}

void ParticleManager::InputParticle(wstring name, int count, PTinfo ptInfo)
{
	for (int i = 0; i < count; i++)
	{
		ParticleSystem*psystem = new ParticleSystem(ptInfo.textures, ptInfo.animSpeed);
		psystem->Initialize(ptInfo.velocity, ptInfo.velocityVar, ptInfo.size, ptInfo.startSize, ptInfo.endSize, ptInfo.perSecontNum, ptInfo.lifeTime, ptInfo.startRange, ptInfo.particleCount);
		psystem->Ready();

		psystem->Gravity() = ptInfo.gravity;
		psystem->GParticleType() = ptInfo.type;
		psystem->GParticleOption() = ptInfo.option;

		particleMap[name].push_back(psystem);
	}

	particleIdx[name] = 0;
}


void ParticleManager::Shot(wstring name, D3DXVECTOR3 pos, D3DXVECTOR3 dir)
{
	mIter miter = particleMap.find(name);

	if (miter->second[particleIdx[name]]->GParticleOption() == ParticleSystem::ParticleOption::Once)
		miter->second[particleIdx[name]]->ResetParticle();
	else
		miter->second[particleIdx[name]]->IsEnd() = false;

	miter->second[particleIdx[name]]->SetTargetPos(pos);
	miter->second[particleIdx[name]]->SetTargetVec(dir);

	particleIdx[name]++;

	if ((size_t)particleIdx[name] >= miter->second.size())
		particleIdx[name] = 0;
}

void ParticleManager::UnShot(wstring name)
{
	mIter miter = particleMap.find(name);

	for (size_t i = 0; i < miter->second.size(); i++)
		miter->second[i]->IsEnd() = true;
}


void ParticleManager::Update()
{
	mIter miter = particleMap.begin();
	for (; miter != particleMap.end(); miter++)
	{
		for (size_t i = 0; i < miter->second.size(); i++)
		{
			miter->second[i]->KillParticle();
			if (miter->second[i]->IsEnd())
				continue;
			miter->second[i]->EmitParticles();
		}
	}
}

void ParticleManager::Render()
{
	mIter miter = particleMap.begin();
	for (; miter != particleMap.end(); miter++)
	{
		for (size_t i = 0; i < miter->second.size(); i++)
		{
			miter->second[i]->Render();
		}
	}
}