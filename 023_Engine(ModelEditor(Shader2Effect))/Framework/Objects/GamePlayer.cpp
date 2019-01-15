#include "Framework.h"
#include "GamePlayer.h"

GamePlayer::GamePlayer(GameCharacterSpec * spec, wstring shaderFile, wstring matFile, wstring meshFile)
	: GameUnit(shaderFile, matFile, meshFile)
	, specData(spec)
{
	//	Initialize
	{
		//Name(String::ToWString(ENUM_TO_STRING(specData->UnitType)));
		Life(specData->Life);
		MaxLife(specData->Life);

		debugMode = DebugMode::ENone;
		enableHandleInput = true;
		currentAction = Action::EDefault;
		prepareAction = Action::EDefault;
		isOverWriteAction = false;
		actionElapsedTime = 0.0f;

		Enable(true);
		Visible(true);
	}

	//	Animation
	{
	}

	//	Weapon
	{
	}
}

GamePlayer::~GamePlayer()
{
}

void GamePlayer::Update()
{
	__super::Update();
}

void GamePlayer::Render()
{
	__super::Render();
}