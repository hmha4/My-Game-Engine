#include "stdafx.h"
#include "GameScene.h"
#include "Objects/GamePlayer.h"
#include "Objects/GameEnemy.h"
#include "Objects/GameNPC.h"
#include "Environments/Shadow.h"
#include "Environments/ScatterSky.h"
#include "Environments/Terrain.h"
#include "Viewer/Freedom.h"
#include "Viewer/SmoothFollow.h"
#include "Particles/Fire.h"
#include "Environments/Lake.h"

GameScene::GameScene()
	: sky(NULL), terrain(NULL)
	, player(NULL), boss(NULL)
	, shadow(NULL), camDist(85)
	, camRot(10, -120)
{
	scene = SCENE::TITLE;
}

GameScene::~GameScene()
{
	sky = NULL;
	terrain = NULL;

	SAFE_DELETE(player);
	SAFE_DELETE(npc);
	for (GameEnemy * enemy : enemies)
		SAFE_DELETE(enemy);

	boss = NULL;

	enemies.clear();
	enemies.shrink_to_fit();

	objects.clear();
	objects.shrink_to_fit();

	shadow = NULL;
	SAFE_DELETE(fire);
	SAFE_DELETE(lake);
}

void GameScene::Initialize()
{
}

void GameScene::Ready()
{
}

void GameScene::Update()
{
	sky->Update();
	shadow->Update();
	lake->Update();

	switch (scene)
	{
	case GameScene::SCENE::TITLE:
		
		if (Mouse::Get()->Down(0))
		{
			scene = SCENE::MAIN;
			UIManager::Get()->SetUiRender(L"11GameTitleUI", false);
			player->SetUI(true);
			player->SetRotation();
			Context::Get()->GetMainCamera()->LookAtPosition(player->Position().x, player->Position().y + 4, player->Position().z);
			camDist = 10;
			((SmoothFollow*)Context::Get()->GetMainCamera())->SetDistance(camDist);
		}
		break;
	case GameScene::SCENE::PROLOG:
		break;
	case GameScene::SCENE::MAIN:
		
		if (npc->StageClear() == true)
		{
			scene = SCENE::ENDING;
			UIManager::Get()->SetUiRender(L"14GameClear", true);
			UIManager::Get()->UseFadeOut(2.0f);
			break;
		}

		if (player != NULL)
			player->Update();

		if (Keyboard::Get()->Down('P'))
		{
			for (GameEnemy * enemy : enemies)
				enemy->Life(0);
		}

		break;
	case GameScene::SCENE::ENDING:
		if (UIManager::Get()->EndFadeOut() == true)
			scene = SCENE::END;
		break;
	case GameScene::SCENE::END:
		SceneManager::Get()->ChangeScene(SceneManager::SCENETYPE::EndScene);

		return;

		break;
	default:
		break;
	}
	
	for (GameEnemy * enemy : enemies)
		enemy->Update();

	npc->Update();
}

void GameScene::PreRender()
{
	sky->PreRender();
	shadow->PreRender();
	lake->PreRender();
}

void GameScene::Render()
{
	if(DEFERRED == false)
		sky->Render();
	shadow->Render();

	npc->Render();
}

void GameScene::PostRender()
{
}

void GameScene::ResizeScreen()
{
	shadow->ResizeScreen();
}

void GameScene::InputShadow(Shadow * shadow)
{
	this->shadow = shadow;
}

void GameScene::InitObjects(map<wstring, IGameObject *> gameObjects)
{
	for (pair<wstring, IGameObject *> iter : gameObjects)
	{
		if (iter.second->Tag() == L"Sky")
			sky = reinterpret_cast<ScatterSky*>(iter.second);
		else if (iter.second->Tag() == L"Terrain")
			terrain = (Terrain*)(iter.second);
		else if (iter.second->Tag() == L"Player")
		{
			GameAnimator * model = dynamic_cast<GameAnimator*>(iter.second);

			if (model != NULL)
			{
				player = new GamePlayer(model);
				player->Initialize();
				player->Ready();
				player->InputTerrain(terrain);

				D3DXVECTOR3 pos = player->Position();
				Context::Get()->GetMainCamera()->LookAtPosition(-20, 0, 0);
				Context::Get()->GetMainCamera()->Rotation(Math::ToRadian(camRot.x), Math::ToRadian(camRot.y));
				((SmoothFollow*)Context::Get()->GetMainCamera())->SetDistance(camDist);
			}
		}
		else if (iter.second->Tag() == L"Enemy")
		{
			GameAnimator * model = dynamic_cast<GameAnimator*>(iter.second);

			if (model != NULL)
			{
				GameEnemy * enemy = new GameEnemy(model);
				enemy->Initialize();
				enemy->Ready();
				enemy->InputTerrain(terrain);
				enemy->InputPlayer(player);
				enemy->InputNPC(npc);
				enemies.push_back(enemy);
			}
		}
		else if (iter.second->Tag() == L"NPC")
		{
			GameAnimator * model = dynamic_cast<GameAnimator*>(iter.second);

			if (model != NULL)
			{
				npc = new GameNPC(model);
				npc->Initialize();
				npc->Ready();
			}
		}
		else if (iter.second->Tag() == L"Boss")
		{
			GameAnimator * model = dynamic_cast<GameAnimator*>(iter.second);

			if (model != NULL)
			{
				//TODO: add boss
			}
		}
		else
		{
			GameModel * model = dynamic_cast<GameModel*>(iter.second);

			if (model != NULL)
				objects.push_back(model);
		}
	}

	npc->InputTerrain(terrain);
	npc->InputPlayer(player);
	npc->SetEnemyCount((UINT)enemies.size());

	lake = new Lake(sky, terrain);
	lake->Initialize();
	lake->Ready();
	AlphaRenderer::Get()->AddAlphaMaps((IAlpha*)lake);

	fire = new Fire();
	fire->Initialize();
	AlphaRenderer::Get()->AddAlphaMaps(fire);
	fire->Setting(0, 4, 0.6f, 1.35f, 1.0f, 1.0f);
	fire->Ready(28.157f, 7.6f, 33.537f);
}

void GameScene::ClearObjects()
{
	sky = NULL;
	terrain = NULL;
	player->Clear();
	SAFE_DELETE(player);
	npc->Clear();
	SAFE_DELETE(npc);
	boss = NULL;

	for (GameEnemy * enemy : enemies)
	{
		enemy->Clear();
		SAFE_DELETE(enemy);
	}
	enemies.clear();
	enemies.shrink_to_fit();

	objects.clear();
	objects.shrink_to_fit();
}
