#pragma once
#include "GameUnit.h"

class GameNPC : public GameUnit
{
public:
	GameNPC(GameAnimator * model);
	~GameNPC();

	void Initialize();
	void Ready();
	void TitleUpdate();
	void Update();
	void Render();
	void Clear();

	void InputTerrain(class Terrain * terrain);
	void InputPlayer(class GamePlayer * player);
	UINT GetEnemyCount() { return enemyCount; }
	void SetEnemyCount(UINT count) { enemyCount = count; }

	bool StageClear() { return clear; }

private:
	enum class STATE
	{
		DEFAULT,
		IDLE,
		IDLE2,
		SIT,
		RUN,
		WAVE
	} state;

private:
	Terrain * terrain;
	GamePlayer * player;

	float distance;
	bool isTalk;

	UINT uiIndex;
	UINT enemyCount;

	D3DXVECTOR3 rotFrom;
	D3DXVECTOR3 rotTo;
	float turnTime;

	int rotFactor;
	bool dialog13;

	bool clear;

};