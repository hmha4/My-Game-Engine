#pragma once
#include "GameUnit.h"

class GamePlayer : public GameUnit
{
public:
	/// <summary>
	/// player's Actions
	/// </summary>
	enum class Action
	{
		EDefault = 0,
		EIdle,
		EWalk, EWalkBackword,
		ERun,
		EDamage,
		EDead,
		Count
	};

	enum class DebugMode
	{
		ENone = 0,	//	Normal play
		ENeverDie,	//	Never dies
		ESuperMan,	//	One shot one kill
		EGod,		//	ENeverDie + ESuperMan
		Count
	};

public:
	GamePlayer(GameCharacterSpec * spec, wstring shaderFile, wstring matFile, wstring meshFile);
	~GamePlayer();

	void Update();
	void Render();

public:
	GameCharacterSpec * SpecData() { return specData; }

	DebugMode GetDebugMode() { return debugMode; }
	Action GetAction() { return currentAction; }

	void EnableHandleInput(bool val) { enableHandleInput = val; }
	bool EnableHandleInput() { return enableHandleInput; }

	//bool IsTryEmptyWeapon() { return isTryEmptyWeapon; }

	bool IsFinishedDead() { return IsDead() && actionElapsedTime; }
	//bool IsWeaponChanging() { return currentAction == Action::EWeaponChange && prepareAction == Action::EWeaponChange; }

private:
	GameCharacterSpec * specData;

	DebugMode debugMode;

	bool enableHandleInput;

	Action currentAction;
	Action prepareAction;

	bool isOverWriteAction;
	float actionElapsedTime;

	vector<UINT> animIndices;

	class GameWeapon * possiblePickUpWeapon;
	bool isTryEmptyWeapon;

	D3DXVECTOR3 moveDirection;
};