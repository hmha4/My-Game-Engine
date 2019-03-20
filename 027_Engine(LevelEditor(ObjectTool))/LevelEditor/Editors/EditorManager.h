#pragma once


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

	map<wstring, class IGameObject *> GameObjects() { return gameObjects; }
	class Shadow * GetShadow() { return shadow; }
private:
	void Reset();
	void Open();
	void Save();

	void RenderHeirarchy();
	void RenderProject();
	void RenderInspector();
	void RenderAnimation();

private:
	void CreateTerrain(wstring fileName);
	void LoadObject(wstring fileName);
	void LoadObjectTransform(wstring fileName, D3DXVECTOR3& p, D3DXVECTOR3& r, D3DXVECTOR3& s);
	void LoadMesh(UINT type);

private:
	//	Basic
	class Shadow * shadow;
	class ScatterSky * sky;
	//class Gizmo * gizmo;

	//	Tool ฐüทร
	EditorType curToolType;
	map<EditorType, class GameEditor *> editors;
	typedef map<EditorType, class GameEditor*>::iterator toolIter;

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
	Effect * modelEffect;
	Effect * animEffect;
	Effect * lineEffect;

	//	ImGui
	ImGuiTextFilter	filter;
	int selection_mask;
	
};