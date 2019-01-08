#pragma once
#include "MapEditor.h"

class EditorManager
{
public:
	enum class EditorType
	{
		NONE,
		OBJECTTOOL,
		TERRAINTOOL,
		CAMERATOOL,
		SKYTOOL
	};
public:
	EditorManager();
	~EditorManager();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void PostRender();

private:
	void CreateTerrain(wstring fileName);

	void RenderHeirarchy();
	void RenderProject();
	void RenderInspector();
	void RenderAnimation();

private:
	//	Basic
	class ScatterSky * sky;


	//	Tool ฐüทร
	EditorType curToolType;
	map<EditorType, GameEditor *> editors;
	typedef map<EditorType, GameEditor*>::iterator toolIter;


	//	GameObjects
	vector<string> objNames;
	map<wstring, class IGameObject *> gameObjects;
	typedef map<wstring, IGameObject*>::iterator gameObjIter;


	//	Materials


	//	ImGui
	ImGuiTextFilter	filter;


	//	Shadow
	class Shadow * shadow;
};