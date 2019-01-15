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
	void Reset();
	void Open();

	void CreateTerrain(wstring fileName);

	void RenderHeirarchy();
	void RenderProject();
	void RenderInspector();
	void RenderAnimation();

private:
	//	Basic
	class Shadow * shadow;
	class ScatterSky * sky;
	class Gizmo * gizmo;


	//	Tool ฐüทร
	EditorType curToolType;
	map<EditorType, GameEditor *> editors;
	typedef map<EditorType, GameEditor*>::iterator toolIter;


	//	GameObjects
	vector<string> objNames;
	map<wstring, class IGameObject *> gameObjects;
	typedef map<wstring, IGameObject*>::iterator gameObjIter;

	UINT terrainCount;
	UINT cubeCount;
	UINT sphereCount;
	UINT planeCount;
	UINT quadCount;
	UINT cylCount;


	//	Materials
	Material * meshMaterial;

	//	ImGui
	ImGuiTextFilter	filter;
};