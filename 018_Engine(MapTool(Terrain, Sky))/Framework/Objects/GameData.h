#pragma once

enum UnitType
{
	EUnknown,

	//	Player
	EPlayer1,
	EPlayer2,
	EPlayer3,

	//	Enemy
	EZombie1,
	EZombie2,
	EZombie3,

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
	//	파일이 있는 폴더 경로 + 파일 이름
	wstring SourceFilePath;
};
#pragma endregion

#pragma region GameCharacterSpec
/// <summary>
/// It has a number that shows the player’s capabilities 
/// and is used as reference during game play. 
/// The class reads from a player spec file(Content/Data/Players/<*>.spec) 
/// and stores the values.
/// The Player spec file is in XML format and the values can be easily changed.
/// </summary>
struct GameCharacterSpec : public GameDataSpec
{
	GameCharacterSpec() {}
	~GameCharacterSpec() {}

	UnitType UnitType = UnitType::EUnknown;

	UINT Life = 0;

	float Radius = 0.0f;
	float RunSpeed = 0.0f;
	float WalkSpeed = 0.0f;
	float WalkBackwardSpeed = 0.0f;

	wstring ModelFilePath = L"";			//	파일이 있는 폴더 경로까지
	wstring AnimationFolderPath = L"";		//	파일이 있는 폴더 경로까지
	wstring DefaultWeaponFilePath = L"";	//	파일이 있는 폴더 경로까지

	D3DXVECTOR3 CameraTargetOffset = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 CameraPositionOffset = D3DXVECTOR3(0, 0, 0);
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
	wstring PlayerSpec = L"";

	UINT EnemyCount = 0;
	vector<wstring> EnemySpecs;

	UINT ObjectCount = 0;
	vector<wstring> ObjectSpecs;
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
private:
	static GameDataSpecManager * instance;

	map<wstring, GameDataSpec *> specList;
	map<wstring, GameDataSpec *>::iterator specListIter;
};