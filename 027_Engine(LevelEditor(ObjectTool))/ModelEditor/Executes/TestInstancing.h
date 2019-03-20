#pragma once
#include "Systems/IExecute.h"

class TestInstancing : public IExecute
{
public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	//vector<GameAnimator*> models;
	GameAnimator * model;
	//GameModel * model;
	Effect * effect;

	class TrailRenderer * trail;
	Effect * trailEffect;

};