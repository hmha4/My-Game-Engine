#pragma once

class ModelBone;

class GameRender
{
public:
	GameRender();
	virtual ~GameRender();

	virtual void Update();
	virtual void Render();

	// --------------------------------------------------------------------- //
	//  Enable for Update
	// --------------------------------------------------------------------- //
	void Enable(bool val);	//	Set Enable
	bool Enable();			//	Get Enable
	vector<function<void(bool)>> Enabled;

	// --------------------------------------------------------------------- //
	//  Visible for Render
	// --------------------------------------------------------------------- //
	void Visible(bool val);	//	Set Visible
	bool Visible();			//	Get Visible
	vector<function<void(bool)>> Visibled;

	// --------------------------------------------------------------------- //
	//  Root Axis
	// --------------------------------------------------------------------- //
	void RootAxis(D3DXMATRIX& matrix);			//	Set root axis by matrix
	void RootAxis(D3DXVECTOR3& rotation);		//	Set root axis rotation
	void RootAxis(float x, float y, float z);	//	Set root axis rotation
	D3DXMATRIX RootAxis();						//	Get root axis

	// --------------------------------------------------------------------- //
	//  World Matrix
	// --------------------------------------------------------------------- //
	D3DXMATRIX World();			//	World
	D3DXMATRIX Transformed();	//	RootAxis * World

	// --------------------------------------------------------------------- //
	//  Transform
	// --------------------------------------------------------------------- //
	virtual void Position(D3DXVECTOR3& vec);			//	Set position of the model
	virtual void Position(float x, float y, float z);	//	Set position of the model
	virtual D3DXVECTOR3 Position();						//	Get position of the model

	virtual void Scale(D3DXVECTOR3& vec);				//	Set scale of the model
	virtual void Scale(float x, float y, float z);		//	Set scale of the model
	virtual D3DXVECTOR3 Scale();						//	Get scale of the model

	virtual void Rotation(D3DXVECTOR3& vec);				//	Set rotation(radian) of the model
	virtual void Rotation(float x, float y, float z);		//	Set rotation(radian) of the model
	virtual void RotationDegree(D3DXVECTOR3& vec);			//	Set rotation(degree) of the model
	virtual void RotationDegree(float x, float y, float z);	//	Set rotation(degree) of the model
	virtual D3DXVECTOR3 Rotation();							//	Get rotation(radian) of the model
	virtual D3DXVECTOR3 RotationDegree();					//	Get rotation(degree) of the model

	// --------------------------------------------------------------------- //
	//  Direction
	// --------------------------------------------------------------------- //
	D3DXVECTOR3 Forward();	//	Forward direction
	D3DXVECTOR3 Up();		//	Up direction
	D3DXVECTOR3 Right();	//	Right direction

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

	bool IsPicked();
	ColliderElement * Collider() { return collider; }
protected:
	// --------------------------------------------------------------------- //
	//  Calculate the world matrix and direction
	// --------------------------------------------------------------------- //
	virtual void UpdateWorld();

protected:
	bool			isChild;		//	Determines if it is a child
	wstring			parentName;		//	Parent model name
	ModelBone*		parentBone;		//	Pointer to the parent bone
	D3DXMATRIX		parentBoneWorld;//	Parent bone world

	bool			enable;			//	Determines if it is enabled
	bool			visible;		//	Determines if it is visible

	BBox * pickCollider;
	ColliderElement * collider;
private:
	D3DXMATRIX rootAxis;			//	Root matrix of the model
	D3DXMATRIX world;				//	World matrix of the model

	D3DXVECTOR3 position;			//	Position of the model
	D3DXVECTOR3 scale;				//	Scale of the model
	D3DXVECTOR3 rotation;			//	Rotation of the model

	D3DXVECTOR3 forward;			//	Forward direction of the model
	D3DXVECTOR3 up;					//	Up direction of the model
	D3DXVECTOR3 right;				//	Right direction of the model
};