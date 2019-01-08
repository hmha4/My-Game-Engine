#pragma once
#include "GameEditor.h"

class MapEditor : public GameEditor
{
public:
	///<sumarry>
	///	enum class for terrain brush
	///</summary>
	enum class MAP_BRUSH
	{
		None,		//	does nothing
		UpBrush,	//	adjusts(up) terrain height
		DownBrush,	//	adjusts(down) terrain height
		BlendBrush	//	splats terrain texture
	};

	///<sumarry>
	///	enum class for terrain setting types
	///</summary>
	enum class MAP_SETTYPE
	{
		SET_NONE,			//	does nothing
		SET_DETAIL_TERRAIN,	//	settings about detail map
		SET_BRUSH_TERRAIN,	//	settings about brush
		SET_BLEND_TERRAIN,	//	settings about blend map
		SET_FOG_TERRAIN,	//	settings about fog
		SET_PLANTS_TERRAIN	//	settings about plants
	};

	///<sumarry>
	///	enum class for terrain blend textures
	///</summary>
	enum class MAP_BLENDTYPE
	{
		NONE,
		BLEND1,
		BLEND2,
		BLEND3
	};

public:
	MapEditor();
	~MapEditor();

	// GameEditor을(를) 통해 상속됨
	void Initalize() override;
	void Ready() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void HierarchyRender() override;
	void ProjectRender() override;
	void InspectorRender() override;
	void AssetRender() override;
	void Delete() override;

	void SetGameObjectPtr(IGameObject * gameObject) override;
	void SetType(UINT type) override;

private:
	void SelectSetType();
	void BasicSetRender();

	void LoadTerrainMapFile(wstring filename, int type);
	void LoadHeightMapFile(wstring file);
	void LoadBlendMapFile(wstring file, int bNum);
	void LoadGrassMapFile(wstring filename, int index);

	void CreateComputeTexture();

private:
	class Terrain * terrain;
	class TerrainRenderer * renderer;

	MAP_SETTYPE mSetType = MAP_SETTYPE::SET_NONE;
	MAP_BRUSH mBrushType = MAP_BRUSH::None;
	MAP_BLENDTYPE mBlendType = MAP_BLENDTYPE::NONE;

	int materialIndex = -1;
	string errorMsg;
private:
	Shader * brushShader;
	ID3D11ShaderResourceView*srv;
	ID3D11UnorderedAccessView*uav;

	Texture * layerImg[3];
};