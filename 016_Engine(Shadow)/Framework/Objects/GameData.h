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
	UnitType UnitType;

	UINT Life;

	float Radius;
	float RunSpeed;
	float WalkSpeed;
	float WalkBackwardSpeed;

	wstring ModelFilePath;			//	파일이 있는 폴더 경로까지
	wstring AnimationFolderPath;	//	파일이 있는 폴더 경로까지
	wstring DefaultWeaponFilePath;	//	파일이 있는 폴더 경로까지

	D3DXVECTOR3 CameraTargetOffset;
	D3DXVECTOR3 CameraPositionOffset;

	GameCharacterSpec()
	{
		UnitType = UnitType::EUnknown;
		Life = 0;
		Radius = 0.0f;
		RunSpeed = 0.0f;
		WalkSpeed = 0.0f;
		WalkBackwardSpeed = 0.0f;

		ModelFilePath = L"";
		AnimationFolderPath = L"";
		DefaultWeaponFilePath = L"";

		CameraTargetOffset = D3DXVECTOR3(0, 0, 0);
		CameraPositionOffset = D3DXVECTOR3(0, 0, 0);
	}
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

	/// <summary>
	/// loads a game data from file.
	/// </summary>
	/// <param name="file">folder + file name</param>
	/// <returns></returns>
	GameDataSpec * Load(wstring file);
	/// <summary>
	/// saves a game data to a file.
	/// </summary>
	/// <param name="folderName">folder name</param>
	/// <param name="fileName">file name</param>
	/// <returns></returns>
	void Write(wstring saveFolder, wstring fileName, GameCharacterSpec * spec);
	
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