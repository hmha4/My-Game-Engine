#pragma once
#include "GameUnit.h"

class GamePlayer : public GameUnit
{
public:
	GamePlayer(GameAnimator * model);
	~GamePlayer();

	void Initialize();
	void Ready();
	void TitleUpdate();
	void Update();
	void Render();
	void Clear();

	void InputTerrain(class Terrain * terrain);
	void SetRotation();
	void IsTalkable(bool val) { isTalkable = val; }
	bool IsTalkable() { return isTalkable; }
	bool IsTalk() { return isTalk; }
	void IsTalk(bool val);

	void SetUI(bool val);
private:
	void SetParticles();
	void PlaySwordSlash();
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
	class MyBehaviourTree * bt;
	class Terrain * terrain;
	
	bool isMoved;
	float moveTime;

	UINT mouseClickCnt;
	float comboTime;

	UINT skillIndex;

	float stamina;
	bool isTalkable;
	bool isTalk;
private:
	ColliderElement * weaponCol;
	ColliderElement * skill3Col;
	ColliderElement * skill4Col;

	ModelClip *skill1;
	ModelClip *skill2;
	ModelClip *skill3;
	ModelClip *skill4;
	ModelClip *hit;
	ModelClip *idleAlert;
	ModelClip *idle;
	ModelClip *idle2;
	ModelClip *run;
	ModelClip *walk;
	ModelClip *walkBack;
	ModelClip *walkLeft;
	ModelClip *walkRight;
	ModelClip *attack1;
	ModelClip *attack2;
};