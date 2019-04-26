#include "Framework.h"
#include "GameUnit.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderSphere.h"
#include "Collider/ColliderRay.h"
#include "Collider/ColliderElement.h"

GameUnit::GameUnit(GameAnimator * model)
	: model(model), life(0), maxLife(0)
	, playerColLayer(NULL), enemyColLayer(NULL), weaponColLayer(NULL)
	, characterColLayer(NULL), worldColLayer(NULL), itemColLayer(NULL)
{
	velocity = D3DXVECTOR3(0, 0, 0);
}

GameUnit::~GameUnit()
{
}

void GameUnit::Initialize()
{
	weaponColLayer = CollisionContext::Get()->GetLayer(L"Collision Weapon");
	itemColLayer = CollisionContext::Get()->GetLayer(L"Collision Item");
	playerColLayer = CollisionContext::Get()->GetLayer(L"Collision Player");
	enemyColLayer = CollisionContext::Get()->GetLayer(L"Collision Enemy");
	enemyWeapColLayer = CollisionContext::Get()->GetLayer(L"Collision Enemy Weapon");
	characterColLayer = CollisionContext::Get()->GetLayer(L"Collision Character");
	worldColLayer = CollisionContext::Get()->GetLayer(L"Collision World");
}

void GameUnit::Reset()
{
	
}

CollisionResult * GameUnit::MoveHitTest(D3DXVECTOR3 velocityAmount)
{
	CollisionResult * result = NULL;

	//  first, test with world
	result = MoveHitTestWithWorld(velocityAmount);
	if (result != NULL)
		return result;

	//  second, test with other units
	result = MoveHitTestWithEnemy(velocityAmount);
	if (result != NULL)
		return result;

	//  third, test with items
	result = MoveHitTestWithItem(velocityAmount);
	if (result != NULL)
		return result;

	return nullptr;
}

CollisionResult * GameUnit::MoveHitTestWithEnemy(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(model->GetMainCollider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = model->Transformed() * temp;
	playerSphere->Transform(T);

	//  checks for the collision with other mech.
	CollisionResult * result = CollisionContext::Get()->HitTest
	(
		playerSphere,
		enemyWeapColLayer,
		CollisionResult::ResultType::ENearestOne
	);

	if (result != NULL)
	{
		//if (0.0f >= result->Distance())
		//{
		ColliderSphere * col = dynamic_cast<ColliderSphere*>(result->DetecedCollider());
		col->IsActive(false);
		return result;
		//}
	}

	return nullptr;
}

CollisionResult * GameUnit::MoveHitTestWithItem(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(model->GetMainCollider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = model->Transformed() * temp;
	playerSphere->Transform(T);

	//  checks for the collision with other mech.
	CollisionResult * result = CollisionContext::Get()->HitTest
	(
		playerSphere,
		itemColLayer,
		CollisionResult::ResultType::ENearestOne
	);

	if (result != NULL)
	{
		if (0.0f >= result->Distance())
		{
			return result;
		}
	}

	return nullptr;
}

CollisionResult * GameUnit::MoveHitTestWithWorld(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(model->GetMainCollider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = model->Transformed() * temp;
	playerSphere->Transform(T);

	// first, check using sphere.
	{
		CollisionResult * result = CollisionContext::Get()->HitTest
		(
			playerSphere,
			worldColLayer,
			CollisionResult::ResultType::ENearestOne
		);

		if (result != NULL)
		{
			if (0.0f >= result->Distance())
			{
				return result;
			}
		}
	}

	//  second, check using ray.
	{
		D3DXVECTOR3 direction = velocityAmount;
		D3DXVec3Normalize(&direction, &direction);

		ColliderRay * ray = new ColliderRay(playerSphere->WorldCenter(), direction);
		ray->Name(playerSphere->Name());

		CollisionResult * result = CollisionContext::Get()->HitTest
		(
			ray,
			worldColLayer,
			CollisionResult::ResultType::ENearestOne
		);
		SAFE_DELETE(ray);

		if (result != NULL)
		{
			if (result->Distance() <= playerSphere->Radius())
				return result;
		}
	}

	return nullptr;
}

CollisionResult * GameUnit::MoveHitTestWithWepon(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(model->GetMainCollider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = model->Transformed() * temp;
	playerSphere->Transform(T);

	//  checks for the collision with other mech.
	CollisionResult * result = CollisionContext::Get()->HitTest
	(
		playerSphere,
		weaponColLayer,
		CollisionResult::ResultType::ENearestOne
	);

	if (result != NULL)
	{
		//if (0.0f >= result->Distance())
		//{
		//ColliderSphere * col = dynamic_cast<ColliderSphere*>(result->DetecedCollider());
		//col->IsActive(false);
		return result;
		//}
	}

	return nullptr;
}
