#include "Framework.h"
#include "GameEnemy.h"
#include "Module/MyBehaviourTree.h"
#include "Viewer/Orbit.h"
#include "Models/ModelClip.h"
#include "Environments/Terrain.h"
#include "GamePlayer.h"

GameEnemy::GameEnemy(GameAnimator * model)
	: GameUnit(model), terrain(NULL), player(NULL)
	, isMoved(false), isAlert(false), isDead(false)
	, mouseClickCnt(0), comboTime(0.0f)
	, skillIndex(0), stateTime(0.0f)
{
	GameCharacterSpec * spec = NULL;
	GameDataSpec * gamedata = NULL;// GameDataSpecManager::Get()->Find(model->FileName() + L".spec");
	if (gamedata == NULL)
		spec = (GameCharacterSpec*)GameDataSpecManager::Get()->LoadCharacter(model->FileName() + L".spec");
	else
		spec = (GameCharacterSpec*)gamedata;

	spec->BT->InputFunctions
	(
		{
			L"ActionIdle",
			L"ActionHit",
			L"FunctionCollision",
			L"ActionTurn",
			L"ActionMove",
			L"NumericAlertDistance",
			L"NumericAttackDistance",
			L"ActionRun",
			L"ActionAttack",
			L"ActionScreamNTurn",
			L"NumericRandom",
			L"ActionSeek",
			L"NumericLifeUnder",
			L"ActionDie",
		},
		{
			bind(&GameEnemy::ActionIdle, this),
			bind(&GameEnemy::ActionHit, this),
			bind(&GameEnemy::FunctionCollision, this),
			bind(&GameEnemy::ActionTurn, this),
			bind(&GameEnemy::ActionMove, this),
			bind(&GameEnemy::NumericAlertDistance, this),
			bind(&GameEnemy::NumericAttackDistance, this),
			bind(&GameEnemy::ActionRun, this),
			bind(&GameEnemy::ActionAttack, this),
			bind(&GameEnemy::ActionScreamNTurn, this),
			bind(&GameEnemy::NumericRandom, this),
			bind(&GameEnemy::ActionSeek, this),
			bind(&GameEnemy::NumericLifeUnder, this),
			bind(&GameEnemy::ActionDie, this),
		}
		);
	spec->BT->BindNodes();

	bt = spec->BT;

	model->Play("Idle", true, 0.2f);

	spawnT = model->Position();
	spawnR = model->Rotation();
	spawnS = model->Scale();

	state = STATE::IDLE;

	weaponCol = CollisionContext::Get()->GetLayer(L"Collision Enemy Weapon")->FindCollider(L"RightHand Collider" + model->Name());
	weaponCol->IsActive(false);

	Life(5);
}

GameEnemy::~GameEnemy()
{
	model->Stop();

	model->Position(spawnT);
	model->Rotation(spawnR);
	model->Scale(spawnS);
}

void GameEnemy::Initialize()
{
	__super::Initialize();
}

void GameEnemy::Ready()
{

}

void GameEnemy::Update()
{
	if (isDead == true) return;

	bt->Update();

	D3DXVECTOR3 pos = model->Position();
	pos.y = terrain->GetHeight(pos.x, pos.z);

	model->Position(pos);
}

void GameEnemy::Render()
{

}

void GameEnemy::InputTerrain(Terrain * terrain)
{
	this->terrain = terrain;
}

void GameEnemy::InputPlayer(GamePlayer * player)
{
	this->player = player;
}

int GameEnemy::NumericRandom()
{
	int rnd = Math::Random(0, 5000);

	return rnd;
}

// ------------------------------------------------------------------------- //
//	Actions
// ------------------------------------------------------------------------- //
int GameEnemy::ActionIdle()
{
	if (model->GetClip("Attack")->IsPlay() == true)
		return false;

	stateTime += Time::Delta();

	if (model->GetClip("Idle")->IsPlay() == false)
	{
		stateTime = 0;
		isAlert = false;
		model->Play("Idle", true, 0.2f);
	}

	if (stateTime >= Math::Random(5, 20))
	{
		float rnd = Math::Random(-Math::PI, Math::PI);

		rotFrom = model->Rotation();
		rotTo = rotFrom;
		rotFrom.y += rnd;

		state = STATE::TURN;
		stateTime = 0.0f;
	}

	return true;
}

int GameEnemy::ActionSeek()
{
	if (model->GetClip("Seek")->IsPlay() == true)
		return true;

	if (model->GetClip("Seek")->IsPlay() == false)
	{
		isAlert = false;
		model->Play("Seek", false, 0.2f);
	}

	return true;
}

int GameEnemy::ActionHit()
{
	if (model->GetClip("Hit")->IsPlay() == true)
	{
		float k = (float)model->CurrKeyframe();
		float c = (float)model->GetClip("Hit")->FrameCount() * 0.25f;
		float t = k / c;
		if (t <= 1.0f)
		{
			D3DXVECTOR3 pos;
			D3DXVec3Lerp(&pos, &model->Position(), &hitBack, t);

			model->Position(pos);
		}

		if (model->CurrKeyframe() > 10)
		{
			CollisionResult * result = MoveHitTestWithWepon(velocity);
			if (result != NULL)
			{
				int life = Life();
				Life(life - 1);

				model->Play("Hit", false, 0.02f);

				D3DXVECTOR3 position = model->Position();
				D3DXVECTOR3 dir = model->Forward();

				velocity = dir * 0.5f;
				hitBack = position + velocity;
			}
		}
		
		return true;
	}

	if (model->GetClip("Hit")->IsPlay() == false)
	{
		int life = Life();
		Life(life - 1);

		model->Play("Hit", false, 0.02f);

		D3DXVECTOR3 position = model->Position();
		D3DXVECTOR3 dir = model->Forward();

		velocity = dir * 0.5f;
		hitBack = position + velocity;

		return true;
	}

	return false;
}

int GameEnemy::ActionTurn()
{
	if (state != STATE::TURN)
		return false;

	if (model->GetClip("Walk")->IsPlay() == false)
	{
		model->Play("Walk", true, 0.2f);
		isAlert = false;
		return true;
	}

	stateTime += Time::Delta();

	if (stateTime <= 1.0f)
	{
		D3DXVECTOR3 rot = model->Rotation();
		D3DXVec3Lerp(&rot, &rotTo, &rotFrom, stateTime);
		model->Rotation(rot);

		return true;
	}

	if (stateTime >= 1.0f)
	{
		state = STATE::MOVE;
		stateTime = 0.0f;
	}

	return false;
}

int GameEnemy::ActionMove()
{
	if (state != STATE::MOVE)
		return false;
	if (model->GetClip("Attack")->IsPlay() == true)
		return false;

	if (model->GetClip("Walk")->IsPlay() == false)
	{
		isAlert = false;
		model->Play("Walk", true, 0.2f);
		return true;
	}

	stateTime += Time::Delta();

	if (stateTime <= 5.0f)
	{
		D3DXVECTOR3 dir = model->Forward();
		velocity = -dir * 0.5f * Time::Delta();

		D3DXVECTOR3 position = model->Position();
		model->Position(position + velocity);

		return true;
	}

	if (stateTime >= 5.0f)
	{
		state = STATE::IDLE;
		stateTime = 0.0f;
	}

	return false;
}

int GameEnemy::NumericAlertDistance()
{
	D3DXVECTOR3 myPos = model->Position();
	D3DXVECTOR3 playerPos = player->Position();

	int distance = (int)Math::GetDistanceXZ(myPos, playerPos);
	
	return distance;
}

int GameEnemy::NumericAttackDistance()
{
	D3DXVECTOR3 myPos = model->Position();
	D3DXVECTOR3 playerPos = player->Position();

	int distance = (int)Math::GetDistanceXZ(myPos, playerPos);

	return distance;
}

int GameEnemy::ActionRun()
{
	if (model->GetClip("Attack")->IsPlay() == true)
		return false;

	if (model->GetClip("Run")->IsPlay() == false &&
		model->GetClip("Attack")->IsPlay() == false)
	{
		model->Play("Run", true, 0.2f);
		return true;
	}

	rotFrom = model->Rotation();

	float angle = Math::GetAngleY(model->Position(), player->Position());
	rotTo = rotFrom;
	rotTo.y = angle - Math::PI_DIV_2;
	model->Rotation(rotTo);

	D3DXVECTOR3 dir = model->Forward();
	velocity = -dir * 2.0f * Time::Delta();

	D3DXVECTOR3 position = model->Position();
	model->Position(position + velocity);

	return true;
}

int GameEnemy::ActionAttack()
{
	if (model->GetClip("Attack")->IsPlay() == true)
	{
		int k = model->CurrKeyframe();
		if (k == 30)
			weaponCol->IsActive(true);
		else if (k > 50)
			weaponCol->IsActive(false);

		return true;
	}

	if (model->GetClip("Attack")->IsPlay() == false)
	{
		model->Play("Attack", false, 0.02f);
		return true;
	}

	return false;
}

int GameEnemy::ActionScreamNTurn()
{
	if (model->GetClip("Attack")->IsPlay() == true)
		return false;

	if (model->GetClip("Alert")->IsPlay() == true)
	{
		float k = (float)model->CurrKeyframe();
		float f = (float)model->GetClip("Alert")->FrameCount();
		float t = k / f;
		t *= 4.0f;

		if (t <= 1.0f)
		{
			D3DXVECTOR3 rot;
			D3DXVec3Lerp(&rot, &rotFrom, &rotTo, t);

			model->Rotation(rot);
		}
		
		return true;
	}

	if (isAlert == true)
		return false;

	if (model->GetClip("Alert")->IsPlay() == false)
	{
		isAlert = true;
		model->Play("Alert", false, 0.2f);
		
		rotFrom = model->Rotation();
		D3DXVECTOR3 playerPos = player->Position();
		D3DXVECTOR3 enemyPos = model->Position();
		playerPos.y = 0.0f;
		enemyPos.y = 0.0f;
		
		rotTo = playerPos - enemyPos;
		D3DXVECTOR3 dir = -model->Forward();
		float rotation = Math::GetAngle(dir, rotTo);

		rotTo = rotFrom;
		rotTo.y += rotation;
		
		return true;
	}

	

	return false;
}

int GameEnemy::NumericLifeUnder()
{
	return Life();
}

int GameEnemy::ActionDie()
{
	if (model->CurrKeyframe() == model->GetClip("Die")->FrameCount() - 2)
	{
		isDead = true;
		return true;
	}

	if (model->GetClip("Die")->IsPlay() == true)
		return true;

	if (model->GetClip("Die")->IsPlay() == false)
	{
		model->Play("Die", false, 0.2f);
		return true;
	}

	return false;
}

int GameEnemy::FunctionCollision()
{
	if (model->GetClip("Hit")->IsPlay() == true)
		return true;

	CollisionResult * result = MoveHitTestWithWepon(velocity);
	if (result != NULL)
	{
		return true;
	}

	return false;
}
