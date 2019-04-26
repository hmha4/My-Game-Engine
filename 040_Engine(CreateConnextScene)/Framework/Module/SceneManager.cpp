#include "Framework.h"
#include "SceneManager.h"

SceneManager*SceneManager::instance = NULL;

void SceneManager::Create()
{
	if (instance == NULL)
	{
		instance = new SceneManager();
		instance->sType = SCENETYPE::MainScene;
		instance->preSceneType = SCENETYPE::LoadingScene;

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
	scenes[(int)sType]->Initialize();
}

void SceneManager::Update()
{
	scenes[(int)sType]->Update();
}

void SceneManager::Ready()
{
	scenes[(int)sType]->Ready();

	ParticleInit();
	UIInit();
	SoundInit();
}

void SceneManager::PreRender()
{
	scenes[(int)sType]->PreRender();
}

void SceneManager::Render()
{
	scenes[(int)sType]->Render();
}

void SceneManager::PostRender()
{
	scenes[(int)sType]->PostRender();
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


	scenes[(int)changeSceneName]->Initialize();
	scenes[(int)changeSceneName]->Ready();

	sType = changeSceneName;
	preSceneType = sType;
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
	//soundManager::Get()->addSound("00startScene", Sounds + "StartScene.wav", true, true);
}