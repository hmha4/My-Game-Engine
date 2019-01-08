#pragma once
#include "Environments/IShadow.h"
#include "Objects/IGameObject.h"

class Mesh : public IShadow, IGameObject
{
public:
	Mesh(Material* material);
	virtual ~Mesh();

	void Render();
	void RenderShadow(UINT tech, UINT pass);

	void Pass(UINT val) { pass = val; }
	UINT Pass() { return pass; }

	Material * GetMaterial() { return material; }

	

protected:
	virtual void CreateData() = 0;
	void CreateBuffer();

private:
	void UpdateWorld();

protected:
	VertexTextureNormalTangent * vertices;
	UINT* indices;

	UINT vertexCount, indexCount;

private:
	UINT		pass;

	wstring name;
	wstring tag;

	D3DXVECTOR3 position;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 rotation;
	D3DXVECTOR3 forward;
	D3DXVECTOR3 up;
	D3DXVECTOR3 right;

	Material* material;
	MaterialProperty::ColorDesc colorDesc;
	MaterialProperty::MaterialMap materialMap;

	D3DXMATRIX world;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	D3DXCOLOR sunColor;

	bool isPicked;

public:
	// IGameObject을(를) 통해 상속됨
	virtual bool IsPicked() override;
	virtual void SetPickState(bool val) override;

	void Position(float x, float y, float z)override;
	void Position(D3DXVECTOR3& vec) override;
	void Position(D3DXVECTOR3* vec) override;
								    
	void Rotation(float x, float y, float z) override;
	void Rotation(D3DXVECTOR3& vec) override;
	void Rotation(D3DXVECTOR3* vec) override;

	void RotationDegree(float x, float y, float z) override;
	void RotationDegree(D3DXVECTOR3& vec) override;
	void RotationDegree(D3DXVECTOR3* vec) override;

	void Scale(float x, float y, float z) override;
	void Scale(D3DXVECTOR3& vec) override;
	void Scale(D3DXVECTOR3* vec) override;

	void Matrix(D3DXMATRIX* mat) override;

	virtual void Forward(D3DXVECTOR3* vec) override;
	virtual void Up(D3DXVECTOR3* vec) override;
	virtual void Right(D3DXVECTOR3* vec) override;
	virtual wstring & Name() override { return name; }
	virtual wstring & Tag() override { return tag; }
	virtual void Save();
	virtual void Load(wstring fileName);
	virtual void Delete();

	// IShadow을(를) 통해 상속됨
	void SetShader(D3DXMATRIX v, D3DXMATRIX p) override;
	void SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	void ShadowUpdate() override;
	void NormalRender() override;
	void ShadowRender(UINT tech, UINT pass) override;
};