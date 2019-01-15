#include "Framework.h"
#include "GameUnit.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderSphere.h"
#include "Collider/ColliderRay.h"
#include "Collider/ColliderElement.h"

GameUnit::GameUnit(wstring shaderFile, wstring matFile, wstring meshFile)
	: GameAnimator(shaderFile, matFile, meshFile)
	, life(0), maxLife(0)
	, playerColLayer(NULL), enemyColLayer(NULL), weaponColLayer(NULL)
	, characterColLayer(NULL), worldColLayer(NULL), itemColLayer(NULL)
{
	D3DXMatrixIdentity(&spawnPoint);
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
	characterColLayer = CollisionContext::Get()->GetLayer(L"Collision Character");
	worldColLayer = CollisionContext::Get()->GetLayer(L"Collision World");
}

void GameUnit::Reset()
{
	D3DXVECTOR3 s, t, r;
	D3DXQUATERNION q;
	D3DXMatrixDecompose(&s, &q, &t, &spawnPoint);
	Math::QuatToYawPithRoll(q, r.x, r.y, r.z);

	Scale(s);
	Rotation(r);
	Position(t);
}

CollisionResult * GameUnit::MoveHitTest(D3DXVECTOR3 velocityAmount)
{
	CollisionResult * result = NULL;

	//  first, test with world
	result = MoveHitTestWithWorld(velocityAmount);
	if (result != NULL)
		return result;

	//  second, test with other units
	result = MoveHitTestWithMech(velocityAmount);
	if (result != NULL)
		return result;

	//  third, test with items
	result = MoveHitTestWithItem(velocityAmount);
	if (result != NULL)
		return result;

	return nullptr;
}

CollisionResult * GameUnit::MoveHitTestWithMech(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(Collider());
	
	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = Transformed() * temp;
	playerSphere->Transform(T);

	//  checks for the collision with other mech.
	CollisionResult * result = CollisionContext::Get()->HitTest
	(
		playerSphere, 
		characterColLayer, 
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

CollisionResult * GameUnit::MoveHitTestWithItem(D3DXVECTOR3 velocityAmount)
{
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(Collider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = Transformed() * temp;
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
	ColliderSphere * playerSphere = dynamic_cast<ColliderSphere *>(Collider());

	D3DXMATRIX temp;
	D3DXMatrixTranslation(&temp, velocityAmount.x, velocityAmount.y, velocityAmount.z);

	D3DXMATRIX T = Transformed() * temp;
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
