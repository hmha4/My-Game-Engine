#pragma once
#include "Framework.h"

class IGameObject
{
public:
	IGameObject() {}
	virtual ~IGameObject() {}
public:
	virtual bool IsPicked() = 0;
	virtual void SetPickState(bool val) = 0;

	virtual void Position(float x, float y, float z) = 0;
	virtual void Position(D3DXVECTOR3& vec) = 0;
	virtual void Position(D3DXVECTOR3* vec) = 0;

	virtual void Rotation(float x, float y, float z) = 0;
	virtual void Rotation(D3DXVECTOR3& vec) = 0;
	virtual void Rotation(D3DXVECTOR3* vec) = 0;

	virtual void RotationDegree(float x, float y, float z) = 0;
	virtual void RotationDegree(D3DXVECTOR3& vec) = 0;
	virtual void RotationDegree(D3DXVECTOR3* vec) = 0;

	virtual void Scale(float x, float y, float z) = 0;
	virtual void Scale(D3DXVECTOR3& vec) = 0;
	virtual void Scale(D3DXVECTOR3* vec) = 0;

	virtual void Matrix(D3DXMATRIX* mat) = 0;

	virtual void Forward(D3DXVECTOR3* vec) = 0;
	virtual void Up(D3DXVECTOR3* vec) = 0;
	virtual void Right(D3DXVECTOR3* vec) = 0;

	virtual wstring & FileName() = 0;
	virtual wstring & Name() = 0;
	virtual wstring & Tag() = 0;
	virtual void Save() = 0;
	virtual void Load(wstring fileName) = 0;
	virtual void Delete() = 0;

	virtual class Model * GetModel() = 0;
	virtual void Clone(void** clone, wstring fileName) = 0;
};