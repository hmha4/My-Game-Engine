#pragma once

class GameScene
{
public:
	GameScene();
	~GameScene();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void PostRender();

	void InputShadow(class Shadow * shadow);

	void InitObjects(map<wstring, class IGameObject *> gameObjects);
	void ClearObjects();
private:

	class ScatterSky * sky;
	class Terrain * terrain;

	class GamePlayer * player;
	class GameBoss * boss;
	vector<class GameEnemy *> enemies;
	vector<GameModel *> objects;

	class Shadow * shadow;
};