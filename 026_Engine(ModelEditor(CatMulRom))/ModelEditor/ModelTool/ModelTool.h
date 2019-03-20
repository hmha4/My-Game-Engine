#pragma once

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

class ModelTool
{
	enum class TARGET
	{
		DEFAULT = 0, BONE, MESH,
	};
public:
	ModelTool();
	~ModelTool();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void Delete();
	void Reset();

private:
	void RenderHeirarchy();
	void RenderProject();
	void RenderInspector();
	void RenderAnimation();

	///<summary>
	/// Transform Functions
	///</summary>
	void Transform(string transform, ModelBone *bone = NULL);

	///<summary>
	/// Child Functions
	///</summary>
	void FixChildObject(int index = -1);

	///<summary>
	/// Bone Functions
	///</summary>
	void GetBoneName(ModelBone *bone);
	void ShowBoneName(int index);
	void InitShowBoneWorld();
	void ShowBoneWorld();
	void PickShowBoneWorld();
	void FixBoneData(int index);

	///<summary>
	/// Mesh Functions
	///</summary>
	void GetMeshData();

	///<summary>
	/// Material Functions
	///</summary>
	void GetMaterialData();
	void FixMaterialData();

	///<summary>
	/// Collider Functions
	///</summary>
	void InitColliderBox(ColliderElement::EType type, wstring colName);
	void RenderColliderBox(GameModel * model);
	void FixColliderBox(int index);

	///<summary>
	/// Trail Functions
	///</summary>
	void InitTrail();
	void FixTrail(int index);
	void SetTrailRun(bool val);
private:
	///<summary>
	/// Load FBX model files from directory
	///</summary>
	void ExportModel(wstring file, UINT type);
	void OpenFBXFile(UINT type);
	///<summary>
	/// Load FBX animation files from directory
	///</summary>
	void ExportAnim(wstring file);
	void OpenAnimFile();
	///<summary>
	/// Load texture files from directory
	///</summary>
	void LoadMaterial(wstring file);
	void OpenMaterialFile();
	///<summary>
	/// Load my .mesh and .material files from directory
	///</summary>
	void LoadMesh(wstring file, UINT type);
	void OpenMeshFile(UINT type);
	///<summary>
	/// Load my .anim files from directory
	///</summary>
	void LoadClip(wstring file);
	void OpenClipFile();

private:
	void Save();
	void Load(wstring fileName);

	void SaveMesh(string objName);
	void SaveMaterial(string objName);
	void WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color);
private:
	class Shadow * shadow;

	//	Effects
	Effect * modelEffect;
	Effect * lineEffect;
	Effect * trailEffect;

	//	Behaviour Objects
	class MyBehaviourTree * currentBT;
	vector<string> BTNames;
	map<wstring, class MyBehaviourTree *> BTs;

	//	Hierarchy Objects
	vector<string> objNames;
	map<wstring, IGameObject *> objects;

	//	Project Asset Objects
	vector<IGameObject*> models;
	vector<Texture *> materials;

	//	Animation Clips
	vector<string> clipNames;
	string currentClipName;

	//	Current Object
	GameModel * currGameModel;
	IGameObject * currObject;
	IGameObject * prevObject;
	Model * model;
	int boneIndex;
	int meshIndex;
	int materialIndex;
	bool showBones;
	bool showBoneInit;
	vector<DebugLine *> showBone;

	//	ImGui
	TARGET target;
	ImGuiTextFilter	filter;
	string errorMessage;
	size_t size;
	Texture * playButton;
	Texture * pauseButton;
	Texture * stopButton;
	D3DXCOLOR pickColor;
	int selection_mask;

	class GizmoGrid * grid;
};