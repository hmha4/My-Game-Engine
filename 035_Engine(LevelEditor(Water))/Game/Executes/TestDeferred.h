#pragma once
#include "Systems/IExecute.h"

class TestDeferred : public IExecute
{
public:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Ready() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override;

private:
	void LoadObjectTransform(wstring fileName, D3DXVECTOR3& p, D3DXVECTOR3& r, D3DXVECTOR3& s);

private:
	class GameScene * gameScene;

	class Shadow * shadow;
	class ScatterSky * sky;
	class Terrain * terrain;

	vector<string> objNames;
	map<wstring, class IGameObject *> gameObjects;
	typedef map<wstring, IGameObject*>::iterator gameObjIter;

	Effect * lineEffect;
	Effect * trailEffect;

	class Fire * fire;
	class Lake * lake;

	//vector<GameAnimator *> models;
	//class MeshGrid * plane;
	//Material * meshMat;
};