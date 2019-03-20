#pragma once
#include "GameUnit.h"

class GamePlayer
{
public:
	GamePlayer(GameAnimator * model);
	~GamePlayer();

	void Initialize();
	void Ready();
	void Update();
	void Render();

	void InputTerrain(class Terrain * terrain);
public:

private:
	int ActionIdle();
	int ActionIdle2();
	int ActionMove();
	int ActionAttack1();
	int NumericRandom();


private:
	GameAnimator * model;
	class MyBehaviourTree * bt;
	class Terrain * terrain;
	
	D3DXVECTOR3 spawnT;
	D3DXVECTOR3 spawnR;
	D3DXVECTOR3 spawnS;

	bool isMoved;
	float moveTime;

	UINT mouseClickCnt;
	float comboTime;

	D3DXMATRIX world;
};