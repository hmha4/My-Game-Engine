#include "stdafx.h"
#include "GameScene.h"
#include "Objects/GamePlayer.h"
#include "Objects/GameEnemy.h"
#include "Environments/Shadow.h"
#include "Environments/ScatterSky.h"
#include "Environments/Terrain.h"
#include "Viewer/Freedom.h"

GameScene::GameScene()
	: sky(NULL), terrain(NULL)
	, player(NULL), boss(NULL)
	, shadow(NULL)
{
	
}

GameScene::~GameScene()
{
	SAFE_DELETE(player);
	for (GameEnemy * enemy : enemies)
		SAFE_DELETE(enemy);
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

	if(player != NULL)
		player->Update();

	for (GameEnemy * enemy : enemies)
		enemy->Update();
}

void GameScene::PreRender()
{
	sky->PreRender();
	shadow->PreRender();
}

void GameScene::Render()
{
	sky->Render();
	shadow->Render();
}

void GameScene::PostRender()
{
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
				enemies.push_back(enemy);
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

	//((Freedom*)Context::Get()->GetMainCamera())->MoveSpeed(0.0f);
	Context::Get()->ChangeCameraType(1);
}

void GameScene::ClearObjects()
{
	sky = NULL;
	terrain = NULL;
	SAFE_DELETE(player);
	boss = NULL;

	for (GameEnemy * enemy : enemies)
		SAFE_DELETE(enemy);
	enemies.clear();
	enemies.shrink_to_fit();

	objects.clear();
	objects.shrink_to_fit();

	//((Freedom*)Context::Get()->GetMainCamera())->MoveSpeed(50.0f);
	Context::Get()->ChangeCameraType(0);
}
