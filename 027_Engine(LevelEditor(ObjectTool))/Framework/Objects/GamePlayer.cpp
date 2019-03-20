#include "Framework.h"
#include "GamePlayer.h"
#include "Module/MyBehaviourTree.h"
#include "Viewer/Orbit.h"
#include "Models/ModelClip.h"
#include "Environments/Terrain.h"

GamePlayer::GamePlayer(GameAnimator * model)
	: model(model), terrain(NULL)
	, isMoved(false), moveTime(0.0f)
	, mouseClickCnt(0), comboTime(0.0f)
{
	GameCharacterSpec * spec = NULL;
	GameDataSpec * gamedata = GameDataSpecManager::Get()->Find(model->FileName() + L".spec");
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
			L"ActionAttack1",
			L"NumericRandom"
		},
		{
			bind(&GamePlayer::ActionIdle, this),
			bind(&GamePlayer::ActionIdle2, this),
			bind(&GamePlayer::ActionMove, this),
			bind(&GamePlayer::ActionAttack1, this),
			bind(&GamePlayer::NumericRandom, this)
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
	
}

void GamePlayer::Ready()
{

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

// ------------------------------------------------------------------------- //
//	Actions
// ------------------------------------------------------------------------- //
int GamePlayer::ActionIdle()
{
	if (isMoved == true)
	{
		if (model->GetClip("Idle_Alart")->IsPlay() == false)
		{
			model->Play("Idle_Alart", true, 0.2f);

			moveTime = 0.0f;
			comboTime = 0.0f;
			mouseClickCnt = 0;

			//ModelBone * bone = model->GetModel()->BoneByIndex(0);
			//D3DXVECTOR3 bonePos = D3DXVECTOR3(bone->World()._41, bone->World()._42, bone->World()._43);
			//D3DXVec3TransformCoord(&bonePos, &bonePos, &model->Transformed());
			//model->Position(bonePos.x, model->Position().y, bonePos.z);
		}

		return true;
	}

	if (model->GetClip("Idle")->IsPlay() == false && 
		model->GetClip("Idle2")->IsPlay() == false)
		model->Play("Idle", true, 0.2f);

	return true;
}

int GamePlayer::ActionIdle2()
{
	if (isMoved == true)
	{
		if (model->GetClip("Idle_Alart")->IsPlay() == false)
		{
			model->Play("Idle_Alart", true, 0.2f);

			moveTime = 0.0f;
			comboTime = 0.0f;
			mouseClickCnt = 0;

			//ModelBone * bone = model->GetModel()->BoneByIndex(0);
			//D3DXVECTOR3 bonePos = D3DXVECTOR3(bone->World()._41, bone->World()._42, bone->World()._43);
			//D3DXVec3TransformCoord(&bonePos, &bonePos, &model->Transformed());
			//model->Position(bonePos.x, model->Position().y, bonePos.z);
		}

		return true;
	}

	if(model->GetClip("Idle2")->IsPlay() == false)
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

		position += -dir * speed * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}
	else if (Keyboard::Get()->Press('S'))
	{
		D3DXVECTOR3 dir = model->Forward();
		D3DXVECTOR3 position = model->Position();

		position += dir * speed * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}

	if (Keyboard::Get()->Press('A'))
	{
		D3DXVECTOR3 dir = model->Right();
		D3DXVECTOR3 position = model->Position();

		position += dir * speed * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}
	else if (Keyboard::Get()->Press('D'))
	{
		D3DXVECTOR3 dir = model->Right();
		D3DXVECTOR3 position = model->Position();

		position += -dir * speed * Time::Delta();

		model->Position(position);

		isMoved = true;
		moveTime = 0.0f;

		return true;
	}

	
	return false;
}

int GamePlayer::ActionAttack1()
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
			if (model->GetClip("Attack1")->IsPlay() == true ||
				model->GetClip("Attack2")->IsPlay() == true)
			{
				bAttack = true;

				/*ModelBone * bone = model->GetModel()->BoneByIndex(0);
				D3DXVECTOR3 bonePos = D3DXVECTOR3(bone->World()._41, bone->World()._42, bone->World()._43);
				D3DXVec3TransformCoord(&bonePos, &bonePos, &world);

				model->Position(bonePos.x, model->Position().y, bonePos.z);*/
			}
			else
				bAttack = false;

			break;
		case 1:
			if (model->GetClip("Attack1")->IsPlay() == true)
				bAttack = true;
			else
			{
				model->Play("Attack1", false, 0.2f);

				isMoved = true;
				bAttack = true;
				moveTime = 0.0f;
				mouseClickCnt = 0;

				//world = model->Transformed();
			}

			break;
		case 2:
			if (model->GetClip("Attack2")->IsPlay() == true)
				bAttack = true;
			else
			{
				model->Play("Attack2", false, 0.2f);

				isMoved = true;
				bAttack = true;
				moveTime = 0.0f;
				mouseClickCnt = 0;

				//world = model->Transformed();
			}

			break;
		}
	}
	
	return bAttack;
}
