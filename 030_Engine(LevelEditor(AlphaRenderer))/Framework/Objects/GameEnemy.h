#pragma once
#include "GameUnit.h"

class GameEnemy : public GameUnit
{
public:
	GameEnemy(GameAnimator * model);
	~GameEnemy();

	void Initialize();
	void Ready();
	void Update();
	void Render();

	void InputTerrain(class Terrain * terrain);
	void InputPlayer(class GamePlayer * player);
private:
	enum class STATE
	{
		TURN,
		MOVE,
		IDLE
	} state;

private:
	int NumericRandom();
	int ActionIdle();
	int ActionSeek();
	int FunctionCollision();
	int ActionHit();
	int ActionTurn();
	int ActionMove();
	int NumericAlertDistance();
	int NumericAttackDistance();
	int ActionRun();
	int ActionAttack();
	int ActionScreamNTurn();
	int NumericLifeUnder();
	int ActionDie();


private:
	class MyBehaviourTree * bt;
	class Terrain * terrain;
	class GamePlayer * player;

	bool isMoved;
	bool isAlert;
	bool isDead;

	float stateTime;

	D3DXVECTOR3 rotFrom;
	D3DXVECTOR3 rotTo;

	D3DXVECTOR3 hitBack;

private:
	ColliderElement *weaponCol;

	ModelClip *attack;
	ModelClip *idle;
	ModelClip *seek;
	ModelClip *hit;
	ModelClip *walk;
	ModelClip *run;
	ModelClip *alert;
	ModelClip *die;
};