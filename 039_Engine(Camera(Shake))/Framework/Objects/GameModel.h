#pragma once
class GameModel : public GameRender
{
public:
	GameModel() {}
	GameModel(UINT instanceID, Effect * effect, wstring matFile, wstring meshFile);
	virtual ~GameModel();

	virtual void Ready(bool val = true) override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void RenderShadow(UINT tech, UINT pass);

	UINT Pass() { return pass; }
	void Pass(UINT val) { pass = val; }

	class Model * GetModel() override { return model; }
	void Clone(void ** clone, wstring fileName) override;

	virtual void UpdateWorld() override;
	// --------------------------------------------------------------------- //
	//  Effect
	// --------------------------------------------------------------------- //
	void SetEffect(wstring fileName);				//	Set the effect of the model by file name
	void SetEffect(string fileName);				//	Set the effect of the model by file name

	void SetDiffuseMap(wstring fileName);			//	Set diffuse map by file name
	void SetSpecularMap(wstring fileName);			//	Set normal map by file name
	void SetNormalMap(wstring fileName);			//	Set specular map by file name
	void SetDetailMap(wstring fileName);			//	Set detail map by file name

	void SetDiffuse(float r, float g, float b, float a);	//	Set diffuse color
	void SetDiffuse(D3DXCOLOR& color);						//	Set diffuse color
	void SetSpecular(float r, float g, float b, float a);	//	Set specular color
	void SetSpecular(D3DXCOLOR& color);						//	Set specular color
	void SetShininess(float val);							//	Set shininess(exp)

	// --------------------------------------------------------------------- //
	//  Load texture file
	// --------------------------------------------------------------------- //
	void LoadDiffuseMapFromFile(wstring fileName);		//	Load diffuse map from file
	void LoadNormalMapFromFile(wstring fileName);		//	Load normal map from file
	void LoadSpecularMapFromFile(wstring fileName);		//	Load specular map from file

	// --------------------------------------------------------------------- //
	//  Colliders
	// --------------------------------------------------------------------- //
	virtual bool IsPicked() override;
	vector<ColliderElement *>& GetColliders() { return colliders; }
	ColliderElement *& GetMainCollider() { return mainCollider; }
	vector<class TrailRenderer *>& GetTrails() { return trails; }

	// --------------------------------------------------------------------- //
	//  Parent model data
	// --------------------------------------------------------------------- //
	void IsChild(bool val) { isChild = val; }						//	Set if it is a child
	bool IsChild() { return isChild; }								//	Get if it is a child
	void HasChild(bool val) { hasChild = val; }						//	Set if it has a child
	bool HasChild() { return hasChild; }							//	Get if it has a child
	//vector<wstring>& ChildNames() { return childNames; }
	void ParentName(wstring val) { parentName = val; }				//	Set parent model name
	wstring ParentName() { return parentName; }						//	Get parent model name
	void ParentBone(ModelBone *val) { parentBone = val; }			//	Set parent bone
	ModelBone *ParentBone() { return parentBone; }					//	Get parent bone
	void ParentBoneWorld(D3DXMATRIX val) { parentBoneWorld = val; }	//	Set parent bone world
	D3DXMATRIX ParentBoneWorld() { return parentBoneWorld; }		//	Get parent bone world
	void ParentBoneIndex(int index) { parentBoneIndex = index; }
	int ParentBoneIndex() { return parentBoneIndex; }
	void ParentModel(GameModel * model) { parentModel = model; }
	GameModel * ParentModel() { return parentModel; }


	// --------------------------------------------------------------------- //
	//  Behaviour Tree
	// --------------------------------------------------------------------- //
	bool& HasBT() { return hasBT; }
	wstring& BTName() { return btName; }

	// --------------------------------------------------------------------- //
	//  Trail
	// --------------------------------------------------------------------- //
	void SetTrailRun(UINT index, bool val);

	// --------------------------------------------------------------------- //
	//  Shadow
	// --------------------------------------------------------------------- //
	virtual void SetEffect(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv = NULL) override;
	virtual void ShadowUpdate() override;
	virtual void NormalRender() override;
	virtual void ShadowRender(UINT tech, UINT pass) override;

	// --------------------------------------------------------------------- //
	//  Instance
	// --------------------------------------------------------------------- //
	UINT InstanceID() { return instanceID; }
protected:
	virtual void UpdateTransforms();
	

protected:
	ModelInstance * instance;
	Effect * effect;
	UINT instanceID;
	UINT pass;
	bool isInstanced;

	class Model * model;

	ID3D11Buffer * boneBuffer;
	ID3D11ShaderResourceView * boneSRV;

	wstring matFile;
	wstring meshFile;

	D3DXMATRIX *boneTransforms;
	D3DXMATRIX *renderTransforms;

	vector<ID3D11Buffer *> vertexBuffers;
	vector<ID3D11ShaderResourceView *> vertexSRVs;
	vector<ID3D11UnorderedAccessView *> vertexUAVs;

protected:
	BBox * pickCollider;
	ColliderElement * mainCollider;
	vector<ColliderElement *> colliders;

	vector<class TrailRenderer *> trails;

	bool hasBT;
	wstring btName;

protected:
	bool			hasChild;
	bool			isChild;		//	Determines if it is a child
	wstring			parentName;		//	Parent model name
	ModelBone*		parentBone;		//	Pointer to the parent bone
	GameModel*		parentModel;
	D3DXMATRIX		parentBoneWorld;//	Parent bone world
	int				parentBoneIndex;

	//vector<wstring> childNames;

protected:
	ID3DX11EffectMatrixVariable * lightViewVar;
	ID3DX11EffectMatrixVariable * lightProjVar;
	ID3DX11EffectMatrixVariable * shadowTransformVar;
	ID3DX11EffectShaderResourceVariable * shadowMapVar;
};