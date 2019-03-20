#pragma once
#include "GameRender.h"

#include "./Models/ModelMeshPart.h"
#include "./Models/Model.h"
#include "./Models/ModelMesh.h"
#include "./Models/ModelBone.h"

class GameModel : public GameRender
{
public:
	GameModel() {}
	GameModel(wstring effectFile, wstring matFile, wstring meshFile);
	virtual ~GameModel();

	virtual void Ready() override;
	virtual void Update() override;
	virtual void Render() override;

	UINT Pass() { return pass; }
	void Pass(UINT val) { pass = val; }

	class Model * GetModel() override { return model; }
	void Clone(void ** clone, wstring fileName) override;

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
	bool IsPicked();
	vector<ColliderElement *>& GetColliders() { return colliders; }
	vector<class TrailRenderer *>& GetTrails() { return trails; }
	//DebugLine * FindLine(wstring name);		//	Get collider by its name
	//void LineInit(DebugLine *collider);		//	Initialize colliders
	//void LineUpdate(wstring name);			//	Update collider
	//void LineUpdate();						//	Update collider
	//void LineRender();						//	Render collider
	//void LineDelete();						//	Delete collider

	// --------------------------------------------------------------------- //
	//  Parent model data
	// --------------------------------------------------------------------- //
	void IsChild(bool val) { isChild = val; }						//	Set if it is a child
	bool IsChild() { return isChild; }								//	Get if it is a child
	void ParentName(wstring val) { parentName = val; }				//	Set parent model name
	wstring ParentName() { return parentName; }						//	Get parent model name
	void ParentBone(ModelBone *val) { parentBone = val; }			//	Set parent bone
	ModelBone *ParentBone() { return parentBone; }					//	Get parent bone
	void ParentBoneWorld(D3DXMATRIX val) { parentBoneWorld = val; }	//	Set parent bone world
	D3DXMATRIX ParentBoneWorld() { return parentBoneWorld; }		//	Get parent bone world

	virtual void UpdateWorld() override;


	bool& HasBT() { return hasBT; }
	wstring& BTName() { return btName; }

protected:
	void UpdateVertex();

	virtual void UpdateTransforms();
	void MappedBoneBuffer();

protected:
	UINT pass;

	class Model * model;

	ID3D11Buffer * boneBuffer;
	ID3D11ShaderResourceView * boneSRV;

	wstring effectFile;
	wstring matFile;
	wstring meshFile;
	Effect * effect;

	D3DXMATRIX *boneTransforms;
	D3DXMATRIX *renderTransforms;

	vector<ID3D11Buffer *> vertexBuffers;
	vector<ID3D11ShaderResourceView *> vertexSRVs;
	vector<ID3D11UnorderedAccessView *> vertexUAVs;

protected:
	BBox * pickCollider;
	vector<ColliderElement *> colliders;

	vector<class TrailRenderer *> trails;

	bool hasBT;
	wstring btName;

protected:
	bool			isChild;		//	Determines if it is a child
	wstring			parentName;		//	Parent model name
	ModelBone*		parentBone;		//	Pointer to the parent bone
	D3DXMATRIX		parentBoneWorld;//	Parent bone world

protected:
	ID3DX11EffectScalarVariable * boneIndexVariable;
	ID3DX11EffectShaderResourceVariable * boneBufferVariable;
	ID3DX11EffectShaderResourceVariable * vertexVariable;
	ID3DX11EffectUnorderedAccessViewVariable * resultVariable;
};