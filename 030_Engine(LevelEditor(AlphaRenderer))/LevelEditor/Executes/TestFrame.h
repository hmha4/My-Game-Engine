#pragma once
#include "Systems/IExecute.h"

class TestFrame : public IExecute
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
	vector<GameAnimator *> models;
	Effect * effect;

	class MeshGrid * plane;
	Material * meshMat;

	class Shadow * shadow;
};