#pragma once
#include "IGameObject.h"

class ModelBone;

class GameRender : public IGameObject
{
public:
	GameRender();
	virtual ~GameRender();

	virtual void Ready();
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

	virtual D3DXVECTOR3 Position();				//	Get position of the model
	virtual D3DXVECTOR3 Scale();				//	Get scale of the model
	virtual D3DXVECTOR3 Rotation();				//	Get rotation(radian) of the model
	virtual D3DXVECTOR3 RotationDegree();		//	Get rotation(degree) of the model

	// --------------------------------------------------------------------- //
	//  Direction
	// --------------------------------------------------------------------- //
	D3DXVECTOR3 Forward();	//	Forward direction
	D3DXVECTOR3 Up();		//	Up direction
	D3DXVECTOR3 Right();	//	Right direction

protected:
	// --------------------------------------------------------------------- //
	//  Calculate the world matrix and direction
	// --------------------------------------------------------------------- //
	virtual void UpdateWorld();

protected:
	bool			enable;			//	Determines if it is enabled
	bool			visible;		//	Determines if it is visible

private:
	D3DXMATRIX rootAxis;			//	Root matrix of the model
	D3DXMATRIX world;				//	World matrix of the model

	D3DXVECTOR3 position;			//	Position of the model
	D3DXVECTOR3 scale;				//	Scale of the model
	D3DXVECTOR3 rotation;			//	Rotation of the model

	D3DXVECTOR3 forward;			//	Forward direction of the model
	D3DXVECTOR3 up;					//	Up direction of the model
	D3DXVECTOR3 right;				//	Right direction of the model

	wstring fileName;
	wstring name;
	wstring tag;
public:
	// IGameObject을(를) 통해 상속됨
	virtual void SetPickState(bool val) override;

	virtual void Position(float x, float y, float z) override;
	virtual void Position(D3DXVECTOR3 * vec) override;
	virtual void Position(D3DXVECTOR3 & vec) override;

	virtual void Rotation(float x, float y, float z) override;
	virtual void Rotation(D3DXVECTOR3 * vec) override;
	virtual void Rotation(D3DXVECTOR3 & vec) override;

	virtual void RotationDegree(float x, float y, float z) override;
	virtual void RotationDegree(D3DXVECTOR3 * vec) override;
	virtual void RotationDegree(D3DXVECTOR3 & vec) override;

	virtual void Scale(float x, float y, float z) override;
	virtual void Scale(D3DXVECTOR3 & vec) override;
	virtual void Scale(D3DXVECTOR3 * vec) override;

	virtual void Matrix(D3DXMATRIX * mat) override;

	virtual void Forward(D3DXVECTOR3 * vec) override;
	virtual void Up(D3DXVECTOR3 * vec) override;
	virtual void Right(D3DXVECTOR3 * vec) override;

	virtual wstring & FileName() override { return fileName; }
	virtual wstring & Name() override;
	virtual wstring & Tag() override;

	virtual void Save() override;
	virtual void Load(wstring fileName) override;

	virtual void Delete() override;

	virtual class Model * GetModel() override { return NULL; }
	virtual void Clone(void** clone, wstring folderName) override {}
};