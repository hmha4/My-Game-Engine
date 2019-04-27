#pragma once

class GameScene
{
private:
	enum class SCENE
	{
		TITLE,
		PROLOG,
		MAIN,
		ENDING,
		END
	} scene;
public:
	GameScene();
	~GameScene();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

	void InputShadow(class Shadow * shadow);

	void InitObjects(map<wstring, class IGameObject *> gameObjects);
	void ClearObjects();
private:

	class ScatterSky * sky;
	class Terrain * terrain;

	class GamePlayer * player;
	class GameNPC * npc;
	class GameBoss * boss;
	vector<class GameEnemy *> enemies;
	vector<GameModel *> objects;

	class Shadow * shadow;
	class Fire * fire;
	class Lake * lake;

	float camDist;
	D3DXVECTOR2 camRot;

	float bgmVol;
};