#pragma once
#include "Systems/IExecute.h"

class TestMapEditor : public IExecute
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
	enum class Mode
	{
		EDIT,
		GAME
	} mode;

private:
	class EditorManager * editorMgr;
	class GameScene * gameScene;

	Texture * playButton;
	Texture * stopButton;
	int corner;
	float distance;
};