#pragma once

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

enum class ModelType
{
	EUnknown, EGameModel, EGameAnimator,
};

enum UnitType
{
	EUnknown,

	//	Player
	EPlayer1,
	EPlayer2,
	EPlayer3,

	//	Enemy
	EEnemy1,
	EEnemy2,
	EEnemy3,

	Count
};

#pragma region GameDataSpec
/// <summary>
/// game data's base structure.
/// </summary>
struct GameDataSpec
{
	GameDataSpec() {}
	virtual ~GameDataSpec() {}
	//	파일이 있는 폴더 경로
	wstring SourceFilePath;
};
#pragma endregion

#pragma region GameCharacterSpec
struct GameCharacterSpec : public GameDataSpec
{
	GameCharacterSpec() {}
	~GameCharacterSpec();

	ModelType ModelType = ModelType::EUnknown;

	wstring Name = L"";
	wstring Tag = L"";

	struct Collider
	{
		wstring Name = L"";
		UINT Type = 0;
		int BoneIndex = -1;

		D3DXMATRIX RootMatrix;
	};
	UINT ColliderCount;
	vector<Collider> Colliders;

	int IsChild = 0;
	int ParentBoneIndex = -1;
	string ParentName = "";
	D3DXMATRIX RootMatrix;


	UINT ClipCount = 0;
	vector<string> ClipNames;

	class MyBehaviourTree * BT = NULL;
	bool bDelete = false;
};
#pragma endregion

#pragma region GameTerrainSpec
struct GameTerrainSpec : public GameDataSpec
{
	GameTerrainSpec() {}
	~GameTerrainSpec() {}

	wstring HeightMap = L"";

	float HeightScale = 0.0f;
	float HeightMapWidth = 0.0f;
	float HeightMapHeight = 0.0f;

	wstring LayerMapFile[3] = { L"" };

	D3DXVECTOR4 BlendHeight = D3DXVECTOR4(0, 0, 0, 0);
	D3DXVECTOR4 ActiveBlend = D3DXVECTOR4(0, 0, 0, 0);

	float DetailValue = 0.0f;
	float DetailIntensity = 0.0f;

	D3DXCOLOR AmbientColor = D3DXCOLOR(0, 0, 0, 0);

	wstring MaterialValueFile[4] = { L"" };
	D3DXCOLOR DiffuseColor = D3DXCOLOR(0, 0, 0, 0);
	D3DXCOLOR SpecularColor = D3DXCOLOR(0, 0, 0, 0);
	float Shininess = 0.0f;

	int TexScale = 0;
	float FogStart = 0.0f;
	float FogRange = 0.0f;

	//	Grass
	D3DXVECTOR2 WindVelocity;
	UINT GrassCount = 0;

	struct GrassDesc
	{
		wstring GrassDiffuseMap = L"";
		wstring GrassNormalMap = L"";
		wstring GrassSpecularMap = L"";
		wstring GrassDetailMap = L"";

		UINT GrassSize = 0;
		D3DXVECTOR3 GrassPosition = D3DXVECTOR3(0, 0, 0);
		UINT NumOfPatchRows = 0;
		UINT NumOfRootsInPatch = 0;
		UINT NumOfRoots = 0;

		int IsFlower = 0;
		UINT FlowerNum = 0;

		GrassDesc() {}
	};

	vector<GrassDesc> grassDesc;

	//	Billboard
	UINT BillboardCount;

	struct BillboardDesc
	{
		UINT BillType;
		UINT TextureNumber;
		D3DXVECTOR2 Size;
		D3DXVECTOR3 Position;
	};
	vector<BillboardDesc> billDesc;
};
#pragma endregion

#pragma region GameSkySpec
struct GameSkySpec : public GameDataSpec
{
	GameSkySpec() {}
	~GameSkySpec() {}

	wstring LoadStarMap = L"";
	wstring LoadMoonMap = L"";
	wstring LoadGlowMap = L"";
	wstring LoadCloudMap = L"";

	float SetAngle = 0.0f;
	float TimeFactor = 0.0f;
	int SampleCount = 0;

	D3DXVECTOR3 WaveLength = D3DXVECTOR3(0, 0, 0);
	float StartTheta = 0.0f;
};
#pragma endregion

#pragma region GameSceneSpec
struct GameSceneSpec : public GameDataSpec
{
	GameSceneSpec() {}
	~GameSceneSpec() {}

	wstring SkySpec = L"";
	wstring TerrainSpec = L"";

	struct Spec
	{
		wstring Name = L"";
		D3DXVECTOR3 Position = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 Rotation = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 Scale = D3DXVECTOR3(0, 0, 0);
	};

	Spec PlayerSpec;

	UINT EnemyCount = 0;
	vector<Spec> EnemySpecs;

	UINT ObjectCount = 0;
	vector<Spec> ObjectSpecs;

};
#pragma endregion

/// <summary>
/// a manager which manages the game data.
/// Provides an interface to loading game data and
/// the loaded game data is stored in the list.
/// </summary>
class GameDataSpecManager
{
public:
	GameDataSpecManager();
	~GameDataSpecManager();

	static void Create();
	static void Delete();
	static GameDataSpecManager * Get();

	//	캐릭터 관련
	GameDataSpec * LoadCharacter(wstring file);
	void WriteCharacter(wstring fileName, GameCharacterSpec * spec);

	//	씬 관련
	GameDataSpec * LoadScene(wstring file);
	void WriteScene(wstring fileName, GameSceneSpec * spec);

	//	터레인 관련
	GameDataSpec * LoadTerrain(wstring file);
	void WriteTerrain(wstring fileName, GameTerrainSpec * spec);

	//	스카이 관련
	GameDataSpec * LoadSky(wstring file);
	void WriteSky(wstring fileName, GameSkySpec * spec);

	/// <summary>
	/// seaches a game data in the list.
	/// </summary>
	/// <param name="fileName">game data file name</param>
	/// <returns>gama data structure</returns>
	GameDataSpec * Find(wstring fileName);

	void Clear() { specList.clear(); }


	void SetTransforms(class Xml::XMLElement * parent, class Xml::XMLElement * element, class Xml::XMLDocument * document, D3DXMATRIX matrix);
	void GetTransforms(class Xml::XMLElement ** parent, D3DXMATRIX * matrix);
	D3DXMATRIX GetMatrix(D3DXVECTOR3 s, D3DXVECTOR3 r, D3DXVECTOR3 p);
	void GetVectors(D3DXVECTOR3 * s, D3DXVECTOR3 * r, D3DXVECTOR3 * p, D3DXMATRIX w);
private:
	static GameDataSpecManager * instance;

	map<wstring, GameDataSpec *> specList;
	map<wstring, GameDataSpec *>::iterator specListIter;
};