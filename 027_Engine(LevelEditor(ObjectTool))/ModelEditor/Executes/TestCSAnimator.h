#pragma once
#include "Systems/IExecute.h"

class TestCSAnimator : public IExecute
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
	GameAnimator * model;
	class TrailRenderer * trail;
	Effect * trailEffect;
};