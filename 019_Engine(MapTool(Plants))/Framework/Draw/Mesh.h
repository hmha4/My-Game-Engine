#pragma once
#include "Environments/IShadow.h"

class Mesh : public IShadow
{
public:
	Mesh(Material* material);
	virtual ~Mesh();

	void Render();
	void RenderShadow(UINT tech, UINT pass);

	void Position(float x, float y, float z);
	void Position(D3DXVECTOR3& vec);
	void Position(D3DXVECTOR3* vec);

	void Rotation(float x, float y, float z);
	void Rotation(D3DXVECTOR3& vec);
	void Rotation(D3DXVECTOR3* vec);

	void RotationDegree(float x, float y, float z);
	void RotationDegree(D3DXVECTOR3& vec);
	void RotationDegree(D3DXVECTOR3* vec);

	void Scale(float x, float y, float z);
	void Scale(D3DXVECTOR3& vec);
	void Scale(D3DXVECTOR3* vec);

	void Matrix(D3DXMATRIX* mat);

	void Pass(UINT val) { pass = val; }
	UINT Pass() { return pass; }

	Material * GetMaterial() { return material; }

	// IShadow을(를) 통해 상속됨
	void SetShader(D3DXMATRIX v, D3DXMATRIX p) override;
	void SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	void ShadowUpdate() override;
	void NormalRender() override;
	void ShadowRender(UINT tech, UINT pass) override;

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

	D3DXVECTOR3 position;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 rotation;

	Material* material;
	MaterialProperty::ColorDesc colorDesc;
	MaterialProperty::MaterialMap materialMap;

	D3DXMATRIX world;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	D3DXCOLOR sunColor;
};