#include "Framework.h"
#include "SceneManager.h"

SceneManager*SceneManager::instance = NULL;

void SceneManager::Create()
{
	if (instance == NULL)
	{
		instance = new SceneManager();
		instance->sType = SCENETYPE::Default;

		instance->scenes.resize(10);
	}
}

void SceneManager::Delete()
{
	/*for (size_t i = 0; i < instance->scenes.size(); i++)
	SAFE_DELETE(instance->scenes[i]);

	instance->scenes.clear();
	*/
	SAFE_DELETE(instance);
}

void SceneManager::InputScene(SCENETYPE sceneName, IExecute * scene)
{
	scenes[(int)sceneName] = scene;
}

void SceneManager::Init()
{
	SoundInit();
	scenes[(int)sType]->Initialize();
}

void SceneManager::Update()
{
	if (m != NULL)
	{
		bool temp = false;

		m->lock();
		temp = bInitialize;
		m->unlock();

		if (temp == false)
		{
			scenes[(int)SCENETYPE::LoadingScene]->Update();
		}
		else
		{
			t.join();
			SAFE_DELETE(m);
			scenes[(int)SCENETYPE::LoadingScene]->Destroy();

			scenes[(int)sType]->Ready();
			bInitialize = false;
		}
	}
	else
	{
		scenes[(int)sType]->Update();
	}
}

void SceneManager::Ready()
{
	scenes[(int)sType]->Ready();

	ParticleInit();
	UIInit();
}

void SceneManager::PreRender()
{
	if (m != NULL)
	{
		bool temp = false;

		m->lock();
		temp = bInitialize;
		m->unlock();

		if (temp == false)
		{
			scenes[(int)SCENETYPE::LoadingScene]->PreRender();
		}
	}
	else
	{
		scenes[(int)sType]->PreRender();
	}
}

void SceneManager::Render()
{
	if (m != NULL)
	{
		bool temp = false;

		m->lock();
		temp = bInitialize;
		m->unlock();

		if (temp == false)
		{
			scenes[(int)SCENETYPE::LoadingScene]->Render();
		}
	}
	else
	{
		scenes[(int)sType]->Render();
	}
}

void SceneManager::PostRender()
{
	if (m != NULL)
	{
		bool temp = false;

		m->lock();
		temp = bInitialize;
		m->unlock();

		if (temp == false)
		{
			scenes[(int)SCENETYPE::LoadingScene]->PostRender();
		}
	}
	else
	{
		scenes[(int)sType]->PostRender();
	}
}

void SceneManager::Destroy()
{
	for (IExecute * exe : scenes)
	{
		if (exe == NULL) continue;

		exe->Destroy();
		SAFE_DELETE(exe);
	}
	//scenes[curSceneName]->Destroy();
}

void SceneManager::ChangeScene(SCENETYPE changeSceneName)
{
	scenes[(int)sType]->Destroy();

	AlphaRenderer::Get()->RemoveMaps();
	ParticleManager::Get()->Empty();
	LightningManager::Get()->Empty();
	UIManager::Get()->Empty();
	UIManager::Get()->ResetFadeOut();
	InstanceManager::Get()->Empty();
	CollisionContext::Get()->DeleteAllLayer();
	CollisionContext::Get()->AddLayer(L"Collision Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Enemy Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Item");
	CollisionContext::Get()->AddLayer(L"Collision Player");
	CollisionContext::Get()->AddLayer(L"Collision Enemy");
	CollisionContext::Get()->AddLayer(L"Collision Character");
	CollisionContext::Get()->AddLayer(L"Collision World");
	Context::Get()->Empty();

	scenes[(int)SCENETYPE::LoadingScene]->Initialize();
	scenes[(int)SCENETYPE::LoadingScene]->Ready();

	m = new mutex();
	t = thread([&]()
	{
		scenes[(int)changeSceneName]->Initialize();

		m->lock();
		bInitialize = true;
		m->unlock();
	});

	//scenes[(int)changeSceneName]->Ready();

	sType = changeSceneName;
}

void SceneManager::StartLoadingScene()
{
	//thread t;
}

void SceneManager::ParticleInit()
{
	
}

void SceneManager::UIInit()
{
	
}

void SceneManager::SoundInit()
{
	SoundManager::Get()->addSound("00MainBGM", Sounds + "MainBGM.mp3", true, true);
	SoundManager::Get()->addSound("01GameBGM", Sounds + "GameBGM.mp3", true, true);
	SoundManager::Get()->addSound("02FightBGM", Sounds + "FightBGM.mp3", true, true);
	SoundManager::Get()->addSound("03Quest", Sounds + "Quest.mp3", false, false);
	SoundManager::Get()->addSound("04FemaleTalk1", Sounds + "FemaleTalk1.mp3", false, false);
	SoundManager::Get()->addSound("05FemaleTalk2", Sounds + "FemaleTalk2.mp3", false, false);
	SoundManager::Get()->addSound("06FemaleTalk3", Sounds + "FemaleTalk3.mp3", false, false);
	SoundManager::Get()->addSound("07FemaleTalk4", Sounds + "FemaleTalk4.mp3", false, false);
	SoundManager::Get()->addSound("08Clear", Sounds + "Clear.mp3", false, false);
	SoundManager::Get()->addSound("09Skill1", Sounds + "Skill1.mp3", false, false);
	SoundManager::Get()->addSound("10Skill2", Sounds + "Skill2.mp3", false, false);
	SoundManager::Get()->addSound("11PlayerHit", Sounds + "PlayerHit.mp3", false, false);
	SoundManager::Get()->addSound("12Attack1", Sounds + "Attack1.mp3", false, false);
	SoundManager::Get()->addSound("13Attack2", Sounds + "Attack2.mp3", false, false);
	SoundManager::Get()->addSound("14Attack3", Sounds + "Attack3.mp3", false, false);
	SoundManager::Get()->addSound("15SwordHit", Sounds + "SwordHit.mp3", false, false);
	SoundManager::Get()->addSound("16SwordSlash1", Sounds + "SwordSlash1.mp3", false, false);
	SoundManager::Get()->addSound("17SwordSlash2", Sounds + "SwordSlash2.mp3", false, false);
	SoundManager::Get()->addSound("18SwordSlash3", Sounds + "SwordSlash3.mp3", false, false);
	SoundManager::Get()->addSound("19SwordSlash4", Sounds + "SwordSlash4.mp3", false, false);
	SoundManager::Get()->addSound("20SwordSlash5", Sounds + "SwordSlash5.mp3", false, false);
	SoundManager::Get()->addSound("21CampFire", Sounds + "CampFire.mp3", false, true);
	SoundManager::Get()->addSound("22Skill2Effect", Sounds + "Skill2Effect.mp3", false, false);
	SoundManager::Get()->addSound("23Skill1Effect", Sounds + "Skill1Effect.mp3", false, false);
	SoundManager::Get()->addSound("24Footstep", Sounds + "Footstep.mp3", false, false);
	SoundManager::Get()->addSound("25EnemyIdle", Sounds + "EnemyIdle.mp3", false, false);
	SoundManager::Get()->addSound("26EnemyScream", Sounds + "EnemyScream.mp3", false, false);
	SoundManager::Get()->addSound("27EnemyAlert", Sounds + "EnemyAlert.mp3", false, false);
	SoundManager::Get()->addSound("28EnemyDie", Sounds + "EnemyDie.mp3", false, false);
	SoundManager::Get()->addSound("29EnemyAttack", Sounds + "EnemyAttack.mp3", false, false);
	SoundManager::Get()->addSound("30EnemyHit", Sounds + "EnemyHit.mp3", false, false);
	SoundManager::Get()->addSound("31EnemyFootstep", Sounds + "Footstep.mp3", false, false);
}