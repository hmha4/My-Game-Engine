#pragma once

class GameUnit
{
public:
	GameUnit(GameAnimator * model);
	~GameUnit();

#pragma region Initialization
	virtual void Initialize();
	virtual void Reset();
#pragma endregion


#pragma region Properties
public:
	void Life(UINT val) { life = val; }
	UINT Life() { return life; }

	void MaxLife(UINT val) { maxLife = val; }
	UINT MaxLife() { return maxLife; }

	void SpawnPoint(D3DXVECTOR3& t, D3DXVECTOR3& s, D3DXVECTOR3 r)
	{
		spawnT = t;
		spawnS = s;
		spawnR = r;
	}
	D3DXVECTOR3 SpawnPointT() { return spawnT; }
	D3DXVECTOR3 SpawnPointS() { return spawnS; }
	D3DXVECTOR3 SpawnPointR() { return spawnR; }

	D3DXVECTOR3 Position() { return model->Position(); }

	bool IsFullLife() { return life == maxLife; }
	bool IsDead() { 
		return life <= 0;
	}

#pragma endregion

#pragma region Collision Detection
	/// <summary>
	/// checks the collision test, when unit is moving,
	/// against and in order of the other units, world, item.
	/// When there is a collision, returns a result report.
	/// </summary>
	/// <param name="vVelocityAmount"></param>
	/// <returns></returns>
	CollisionResult * MoveHitTest(D3DXVECTOR3 velocityAmount);
	/// <summary>
	/// checks for collision test again other units when the unit is moving.
	/// </summary>
	CollisionResult * MoveHitTestWithEnemy(D3DXVECTOR3 velocityAmount);
	CollisionResult * MoveHitTestWithItem(D3DXVECTOR3 velocityAmount);
	CollisionResult * MoveHitTestWithWorld(D3DXVECTOR3 velocityAmount);
	CollisionResult * MoveHitTestWithWepon(D3DXVECTOR3 velocityAmount);
#pragma endregion


private:
	int life;
	UINT maxLife;

protected:
	GameAnimator * model;
	D3DXVECTOR3 spawnT;
	D3DXVECTOR3 spawnR;
	D3DXVECTOR3 spawnS;

	CollisionLayer * weaponColLayer;
	CollisionLayer * itemColLayer;
	CollisionLayer * playerColLayer;
	CollisionLayer * enemyColLayer;
	CollisionLayer * characterColLayer;
	CollisionLayer * worldColLayer;
	CollisionLayer * enemyWeapColLayer;

	D3DXVECTOR3 velocity;
};