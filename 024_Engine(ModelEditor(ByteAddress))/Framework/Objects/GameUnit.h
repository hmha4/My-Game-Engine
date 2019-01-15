#pragma once

/// <summary>
/// It contains the members that all mech units use and processes the common stuff.
/// It also contains Weapons function and collision layer as a reference.
/// It updates the 3D sound emitter.
/// It also has the members for the number of the unit¡¯s life (HP) and weapon.
/// It also defines the virtual functions for the unit¡¯s action.
/// It provides a basic hit test interface.
/// </summary>
class GameUnit : public GameAnimator
{
public:
	GameUnit(wstring shaderFile, wstring matFile, wstring meshFile);
	~GameUnit();

#pragma region Initialization
	void Initialize();
	void Reset();
#pragma endregion

#pragma region Weapon
public:
	//void CreateWeapon();
	//void SelectWeapon(int slot);
	//GameWeapon * Weapon(int slot) { return weaponList[slot]; }
#pragma endregion

#pragma region Properties
public:
	void Life(UINT val) { life = val; }
	UINT Life() { return life; }

	void MaxLife(UINT val) { maxLife = val; }
	UINT MaxLife() { return maxLife; }

	void SpawnPoint(D3DXMATRIX& val) { spawnPoint = val; }
	D3DXMATRIX SpawnPoint() { return spawnPoint; }

	bool IsFullLife() { return life == maxLife; }
	bool IsDead() { return life <= 0; }

	/// <summary>
	/// About weapons
	/// </summary>
	//bool IsFiring() { return currentWeapon->State == GameWeapon::WeaponState::EFiring; }
	//bool IsReloading() { return currentWeapon->State == GameWeapon::WeaponState::EReloading; }
	//bool IsPossibleWeaponChage() { return (weaponList.size() > 1) && (currentWeapon->State == GameWeapon::WeaponState::EReady); }

	//int CurrentWeaponSlot() { return currentWeaponSlot; }
	//size_t WeaponCount() { return weaponList.size(); }
	//GameWeapon * CurrentWeapon() { return currentWeapon; }
	//GameWeapon * DefaultWeapon() { return weaponList[0]; }
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
	CollisionResult * MoveHitTestWithMech(D3DXVECTOR3 velocityAmount);
	CollisionResult * MoveHitTestWithItem(D3DXVECTOR3 velocityAmount);
	CollisionResult * MoveHitTestWithWorld(D3DXVECTOR3 velocityAmount);
#pragma endregion

#pragma region Action
	virtual void ActionIdle() {}
	virtual bool ActionFire() { return false; }
	virtual bool ActionMelee() { return false; }
	virtual void ActionHit(GameUnit * attacker) {}
	//virtual bool ActionReload(GameWeapon * weapon) {}
	virtual void ActionDamage(GameUnit * attacker) {}
	virtual void ActionDead(D3DXVECTOR3 attackerPos) {}
#pragma endregion

private:
	UINT life;
	UINT maxLife;

	//class GameWeapon * currentWeapon;
	//int currentWeaponSlot;

protected:
	D3DXMATRIX spawnPoint;
	//vector<class GameWeapon *> weaponList;

	CollisionLayer * weaponColLayer;
	CollisionLayer * itemColLayer;
	CollisionLayer * playerColLayer;
	CollisionLayer * enemyColLayer;
	CollisionLayer * characterColLayer;
	CollisionLayer * worldColLayer;
};