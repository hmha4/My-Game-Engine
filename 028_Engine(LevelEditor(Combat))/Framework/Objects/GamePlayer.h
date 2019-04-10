#pragma once
#include "GameUnit.h"

class GamePlayer : public GameUnit
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
	int ActionAttack();
	int NumericRandom();
	int ActionSkill();
	int FunctionCollision();
	int ActionHit();


private:
	//GameAnimator * model;
	class MyBehaviourTree * bt;
	class Terrain * terrain;
	
	bool isMoved;
	float moveTime;

	UINT mouseClickCnt;
	float comboTime;

	UINT skillIndex;

	ColliderElement * weaponCol;
	ColliderElement * skill3Col;
	ColliderElement * skill4Col;
};