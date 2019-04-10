#include "Framework.h"
#include "GamePlayer.h"
#include "Module/MyBehaviourTree.h"
#include "Module/TrailRenderer.h"
#include "Viewer/Orbit.h"
#include "Models/ModelClip.h"
#include "Environments/Terrain.h"
#include "Collider/ColliderSphere.h"

GamePlayer::GamePlayer(GameAnimator * model)
	: GameUnit(model), terrain(NULL)
	, isMoved(false), moveTime(0.0f)
	, mouseClickCnt(0), comboTime(0.0f)
	, skillIndex(0)
{
	GameCharacterSpec * spec = NULL;
	GameDataSpec * gamedata = GameDataSpecManager::Get()->Find(model->Name());
	if (gamedata == NULL)
		spec = (GameCharacterSpec*)GameDataSpecManager::Get()->LoadCharacter(model->FileName() + L".spec");
	else
		spec = (GameCharacterSpec*)gamedata;

	spec->BT->InputFunctions
	(
		{
			L"ActionIdle",
			L"ActionIdle2",
			L"ActionMove",
			L"ActionSkill",
			L"ActionAttack",
			L"NumericRandom",
			L"FunctionCollision",
			L"ActionHit"
		},
		{
			bind(&GamePlayer::ActionIdle, this),
			bind(&GamePlayer::ActionIdle2, this),
			bind(&GamePlayer::ActionMove, this),
			bind(&GamePlayer::ActionSkill, this),
			bind(&GamePlayer::ActionAttack, this),
			bind(&GamePlayer::NumericRandom, this),
			bind(&GamePlayer::FunctionCollision, this),
			bind(&GamePlayer::ActionHit, this),
		}
	);
	spec->BT->BindNodes();

	bt = spec->BT;

	model->Play("Idle", true, 0.2f);

	spawnT = model->Position();
	spawnR = model->Rotation();
	spawnS = model->Scale();
}

GamePlayer::~GamePlayer()
{
	model->Stop();
	
	model->Position(spawnT);
	model->Rotation(spawnR);
	model->Scale(spawnS);
}

void GamePlayer::Initialize()
{
	__super::Initialize();
}

void GamePlayer::Ready()
{
	weaponCol = CollisionContext::Get()->GetLayer(L"Collision Weapon")->FindCollider(L"Main Collider WeaponScottish Sword");
	skill3Col = CollisionContext::Get()->GetLayer(L"Collision Weapon")->FindCollider(L"Skill3 Colliderpaladin_j_nordstrom");
	skill4Col = CollisionContext::Get()->GetLayer(L"Collision Weapon")->FindCollider(L"Skill4 Colliderpaladin_j_nordstrom");
	weaponCol->IsActive(false);
	skill3Col->IsActive(false);
	skill4Col->IsActive(false);

	skill1 = model->GetClip("Skill1");
	skill2 = model->GetClip("Skill2");
	skill3 = model->GetClip("Skill3");
	skill4 = model->GetClip("Skill4");
	hit = model->GetClip("Hit");
	idleAlert = model->GetClip("Idle_Alart");
	idle = model->GetClip("Idle");
	idle2 = model->GetClip("Idle2");
	run = model->GetClip("Run");
	walk = model->GetClip("Walk");
	walkBack = model->GetClip("Walk_Back");
	walkLeft = model->GetClip("Walk_Left");
	walkRight = model->GetClip("Walk_Right");
	attack1 = model->GetClip("Attack1");
	attack2 = model->GetClip("Attack2");
}

void GamePlayer::Update()
{
	if (moveTime <= 5.0f)
		moveTime += Time::Delta();
	else
		isMoved = false;

	bt->Update();

	D3DXVECTOR3 pos = model->Position();
	pos.y = terrain->GetHeight(pos.x, pos.z);

	model->Position(pos);

	D3DXVECTOR3 rot = model->Rotation();

	if (Mouse::Get()->Press(1))
	{
		D3DXVECTOR3 val = Mouse::Get()->GetMoveValue();

		rot.y += val.x * 6.5f* Time::Delta();

		model->Rotation(rot);
	}

	Context::Get()->GetMainCamera()->LookAtPosition(
		pos.x, pos.y + 3, pos.z
	);

}

void GamePlayer::Render()
{
	
}

void GamePlayer::InputTerrain(Terrain * terrain)
{
	this->terrain = terrain;
}

int GamePlayer::NumericRandom()
{
	int rnd = Math::Random(0, 5000);

	return rnd;
}

int GamePlayer::ActionSkill()
{
	if (Keyboard::Get()->Down('1'))
	{
		model->Play("Skill1", false, 0.2f);
		skillIndex = 1;
	}
	else  if (Keyboard::Get()->Down('2'))
	{
		model->Play("Skill2", false, 0.2f);
		skillIndex = 2;
	}
	else if (Keyboard::Get()->Down('3'))
	{
		skill3Col->IsActive(true);
		model->Play("Skill3", false, 0.2f);
		skillIndex = 3;
	}
	else if (Keyboard::Get()->Down('4'))
	{
		skill4Col->IsActive(true);
		model->Play("Skill4", false, 0.2f);
		skillIndex = 4;
	}

	bool bSkill = false;

	switch (skillIndex)
	{
	case 0:
		bSkill = false;
		break;
	case 1:
		if (skill1->IsPlay() == true)
		{
			bSkill = true;

			int currKeyframe = model->CurrKeyframe();
			float t = 0.0f;
			float speed = 2.0f;

			if (currKeyframe <= 30)
			{
				t = (float)currKeyframe / 30.0f;
				speed *= 4;
				weaponCol->IsActive(true);
				model->SetTrailRun(1, true);
			}
			else
			{
				weaponCol->IsActive(false);
				model->SetTrailRun(1, false);
			}

			if (t <= 1)
			{
				D3DXVECTOR3 position = model->Position();
				D3DXVECTOR3 dir = -model->Forward();
				dir *= speed * Time::Delta();

				D3DXVec3Lerp(&position, &position, &(position + dir), t);

				model->Position(position);
			}
		}
		else
		{
			moveTime = 0.0f;
			isMoved = true;
			skillIndex = 0;
		}
		break;
	case 2:
		if (skill2->IsPlay() == true)
		{
			bSkill = true;

			int currKeyframe = model->CurrKeyframe();
			float t = 0.0f;
			float speed = 2.0f;

			if (currKeyframe > 2 && currKeyframe <= 9)
			{
				t = ((float)currKeyframe - 3) / 7.0f;
				speed *= 2;
			}
			else if (currKeyframe > 9 && currKeyframe <= 12)
			{
				t = ((float)currKeyframe - 10) / 3.0f;
				speed *= 4;
			}
			else if (currKeyframe > 12 && currKeyframe <= 37)
			{
				weaponCol->IsActive(true);
				model->SetTrailRun(1, true);
				
				t = ((float)currKeyframe - 13) / 25.0f;
				speed *= 4;
			}
			else
			{
				weaponCol->IsActive(false);
				model->SetTrailRun(1, false);
			}

			if (t <= 1)
			{
				D3DXVECTOR3 position = model->Position();
				D3DXVECTOR3 dir = -model->Forward();
				dir *= speed * Time::Delta();

				D3DXVec3Lerp(&position, &position, &(position + dir), t);

				model->Position(position);
			}
		}
		else
		{
			moveTime = 0.0f;
			isMoved = true;
			skillIndex = 0;
		}
		break;
	case 3:
		if (skill3->IsPlay() == true)
		{
			bSkill = true;
		}
		else
		{
			moveTime = 0.0f;
			isMoved = true;
			skillIndex = 0;
			skill3Col->IsActive(false);
		}
		break;
	case 4:
		if (skill4->IsPlay() == true)
		{
			bSkill = true;
		}
		else
		{
			moveTime = 0.0f;
			isMoved = true;
			skillIndex = 0;
			skill4Col->IsActive(false);
		}
		break;
	}
	
	return bSkill;
}

int GamePlayer::FunctionCollision()
{
	if (hit->IsPlay() == true)
		return true;

	CollisionResult * result = MoveHitTestWithEnemy(velocity);
	if (result != NULL)
	{
		return true;
	}

	return false;
}

int GamePlayer::ActionHit()
{
	if (hit->IsPlay() == true)
		return true;

	if (hit->IsPlay() == false)
	{
		model->Play("Hit", false, 0.02f);
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------- //
//	Actions
// ------------------------------------------------------------------------- //
int GamePlayer::ActionIdle()
{
	if (isMoved == true)
	{
		if (idleAlert->IsPlay() == false)
		{
			model->Play("Idle_Alart", true, 0.2f);

			moveTime = 0.0f;
			comboTime = 0.0f;
			mouseClickCnt = 0;
		}

		return true;
	}

	if (idle->IsPlay() == false && 
		idle2->IsPlay() == false)
		model->Play("Idle", true, 0.2f);

	return true;
}

int GamePlayer::ActionIdle2()
{
	if (isMoved == true)
	{
		if (idleAlert->IsPlay() == false)
		{
			model->Play("Idle_Alart", true, 0.2f);

			moveTime = 0.0f;
			comboTime = 0.0f;
			mouseClickCnt = 0;
		}

		return true;
	}

	if(idle2->IsPlay() == false)
		model->Play("Idle2", false, 0.2f);

	return true;
}

int GamePlayer::ActionMove()
{
	float speed = 2.0f;

	if (Keyboard::Get()->Press(VK_LSHIFT))
	{
		if (Keyboard::Get()->Down('W'))
			model->Play("Run", true, 0.2f);

		speed = speed * 2;
	}
	else
	{
		if (Keyboard::Get()->Down('W'))
			model->Play("Walk", true, 0.2f);
		else if (Keyboard::Get()->Down('S'))
			model->Play("Walk_Back", true, 0.2f);
		if (Keyboard::Get()->Down('A'))
			model->Play("Walk_Left", true, 0.2f);
		else if (Keyboard::Get()->Down('D'))
			model->Play("Walk_Right", true, 0.2f);
	}


	if (Keyboard::Get()->Press('W'))
	{
		if (Keyboard::Get()->Down(VK_LSHIFT))
		{
			model->Play("Run", true, 0.2f);
		}
		else if (Keyboard::Get()->Up(VK_LSHIFT))
		{
			model->Play("Walk", true, 0.2f);
		}
		D3DXVECTOR3 dir = model->Forward();
		D3DXVECTOR3 position = model->Position();
		velocity = -dir * speed;

		position += velocity * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}
	else if (Keyboard::Get()->Press('S'))
	{
		D3DXVECTOR3 dir = model->Forward();
		D3DXVECTOR3 position = model->Position();
		velocity = dir * speed;

		position += velocity * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}

	if (Keyboard::Get()->Press('A'))
	{
		D3DXVECTOR3 dir = model->Right();
		D3DXVECTOR3 position = model->Position();
		velocity = dir * speed;

		position += velocity * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}
	else if (Keyboard::Get()->Press('D'))
	{
		D3DXVECTOR3 dir = model->Right();
		D3DXVECTOR3 position = model->Position();
		velocity = -dir * speed;

		position += velocity * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}

	
	return false;
}

int GamePlayer::ActionAttack()
{
	bool bAttack = false;
	

	if (Mouse::Get()->Down(0))
		mouseClickCnt++;

	if (mouseClickCnt > 0)
		comboTime += Time::Delta();

	if (comboTime < 0.2f)
		bAttack = false;
	else
	{
		switch (mouseClickCnt)
		{
		case 0:
			if (attack1->IsPlay() == true)
			{
				bAttack = true;

				int currKeyframe = model->CurrKeyframe();
				if (currKeyframe > (int)attack1->FrameCount())
					currKeyframe = 0;

				float t = 0.0f;
				float speed = 1.0f;

				if (currKeyframe > 10 && currKeyframe <= 20)
				{
					t = ((float)currKeyframe - 10) / 10.0f;
					if (currKeyframe == 11)
					{
						weaponCol->IsActive(true);
						model->SetTrailRun(0, true);
					}
				}
				else if (currKeyframe > 31 && currKeyframe <= 40)
				{
					t = ((float)currKeyframe - 31) / 9.0f;
					speed *= -1;
					weaponCol->IsActive(false);
					model->SetTrailRun(0, false);
				}

				if (t <= 1)
				{
					D3DXVECTOR3 position = model->Position();
					D3DXVECTOR3 dir = -model->Forward();
					dir *= speed * Time::Delta();

					D3DXVec3Lerp(&position, &position, &(position + dir), t);

					model->Position(position);
				}
			}
			else if(attack2->IsPlay() == true)
			{
				bAttack = true;

				int currKeyframe = model->CurrKeyframe();
				int c = attack2->FrameCount();
				if (currKeyframe > c)
					currKeyframe = 0;

				float t = 0.0f;
				float speed = 2.0f;

				if (currKeyframe > 8 && currKeyframe <= 19)
				{
					t = ((float)currKeyframe - 9) / 10.0f;
					weaponCol->IsActive(true);
					model->SetTrailRun(0, true);
				}
				else if (currKeyframe > 19 && currKeyframe <= 37)
					t = ((float)currKeyframe - 20) / 17.0f;
				else if (currKeyframe > 37 && currKeyframe <= 50)
					t = ((float)currKeyframe - 38) / 12.0f;
				else if (currKeyframe > 50 && currKeyframe <= 69)
					t = ((float)currKeyframe - 51) / 18.0f;
				else if (currKeyframe > 80)
				{
					weaponCol->IsActive(false);
					model->SetTrailRun(0, false);
				}

				if (t <= 1)
				{
					D3DXVECTOR3 position = model->Position();
					D3DXVECTOR3 dir = -model->Forward();
					dir *= speed * Time::Delta();

					D3DXVec3Lerp(&position, &position, &(position + dir), t);

					model->Position(position);
				}
			}
			else
				bAttack = false;

			break;
		case 1:
			if (attack1->IsPlay() == true)
				bAttack = true;
			else
			{
				model->Play("Attack1", false, 0.2f);

				isMoved = true;
				bAttack = true;
				moveTime = 0.0f;
				mouseClickCnt = 0;

			}

			break;
		case 2:
			if (attack2->IsPlay() == true)
				bAttack = true;
			else
			{
				model->Play("Attack2", false, 0.2f);

				isMoved = true;
				bAttack = true;
				moveTime = 0.0f;
				mouseClickCnt = 0;

			}

			break;
		}
	}
	
	return bAttack;
}
