#pragma once

class EndScene : public IExecute
{
private:
	enum class STATE
	{
		DEFAULT,
		FADEIN,
		ENDING,
		FADEOUT
	}state;
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

	void InitUI();
	void InitLights();
	void InitParticles();

private:
	class Fire * fire;
	class Lake * lake;
	class Shadow * shadow;
	class ScatterSky * sky;
	class Terrain * terrain;
	class GameAnimator * player;
	vector<GameModel *> objects;
	vector<string> objNames;

	GameSceneSpec * sceneSpec;

private:
	float offsetX;
	float offset1;
	float offset2;
	float offsetSpeed;
};