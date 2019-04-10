#include "Framework.h"
#include "Mesh.h"

Mesh::Mesh(Material* material)
	: material(material)
	, position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0)
	, vertexBuffer(NULL), indexBuffer(NULL)
	, vertices(NULL), indices(NULL)
	, pass(0)
	, isPicked(false)
{
	D3DXMatrixIdentity(&world);

	worldVariable = material->GetEffect()->AsMatrix("World");
	lightViewVar = material->GetEffect()->AsMatrix("LightView");
	lightProjVar = material->GetEffect()->AsMatrix("LightProjection");
	shadowTransformVar = material->GetEffect()->AsMatrix("ShadowTransform");
	shadowMapVar = material->GetEffect()->AsSRV("ShadowMap");
}

Mesh::~Mesh()
{
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
}

void Mesh::Render()
{
	if (vertexBuffer == NULL && indexBuffer == NULL)
	{
		CreateData();
		CreateBuffer();
	}

	UINT stride = sizeof(VertexTextureNormalTangent);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldVariable->SetMatrix(world);

	material->GetEffect()->DrawIndexed(0, pass, indexCount);
}

void Mesh::RenderShadow(UINT tech, UINT pass)
{
	if (vertexBuffer == NULL && indexBuffer == NULL)
	{
		CreateData();
		CreateBuffer();
	}

	UINT stride = sizeof(VertexTextureNormalTangent);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldVariable->SetMatrix(world);
	material->GetEffect()->DrawIndexed(tech, pass, indexCount);
}

void Mesh::Position(float x, float y, float z)
{
	D3DXVECTOR3 value(x, y, z);

	Position(value);
}

void Mesh::Position(D3DXVECTOR3 & vec)
{
	position = vec;

	UpdateWorld();
}

void Mesh::Position(D3DXVECTOR3 * vec)
{
	*vec = position;
}

void Mesh::Rotation(float x, float y, float z)
{
	D3DXVECTOR3 value(x, y, z);

	Rotation(value);
}

void Mesh::Rotation(D3DXVECTOR3 & vec)
{
	rotation = vec;

	UpdateWorld();
}

void Mesh::Rotation(D3DXVECTOR3 * vec)
{
	*vec = rotation;
}

void Mesh::RotationDegree(float x, float y, float z)
{
	D3DXVECTOR3 value(x, y, z);

	RotationDegree(value);
}

void Mesh::RotationDegree(D3DXVECTOR3 & vec)
{
	rotation = vec * Math::PI / 180.0f;

	UpdateWorld();
}

void Mesh::RotationDegree(D3DXVECTOR3 * vec)
{
	*vec = rotation * 180.0f / Math::PI;
}

void Mesh::Scale(float x, float y, float z)
{
	D3DXVECTOR3 value(x, y, z);

	Scale(value);
}

void Mesh::Scale(D3DXVECTOR3 & vec)
{
	scale = vec;

	UpdateWorld();
}

void Mesh::Scale(D3DXVECTOR3 * vec)
{
	*vec = scale;
}

void Mesh::Matrix(D3DXMATRIX * mat)
{
	*mat = world;
}

void Mesh::Forward(D3DXVECTOR3 * vec)
{
	*vec = forward;
}

void Mesh::Up(D3DXVECTOR3 * vec)
{
	*vec = up;
}

void Mesh::Right(D3DXVECTOR3 * vec)
{
	*vec = right;
}

void Mesh::UpdateWorld()
{
	D3DXMATRIX S, R, T;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationYawPitchRoll(&R, rotation.y, rotation.x, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	world = S * R * T;

	forward = D3DXVECTOR3(world._31, world._32, world._33);
	up = D3DXVECTOR3(world._21, world._22, world._23);
	right = D3DXVECTOR3(world._11, world._12, world._13);
}

bool Mesh::IsPicked()
{
	return isPicked;
}

void Mesh::SetPickState(bool val)
{
	isPicked = val;
}

void Mesh::Save()
{
}

void Mesh::Load(wstring fileName)
{
}

void Mesh::Delete()
{
	if (this != NULL)
		delete this;
}

void Mesh::CreateBuffer()
{
	//Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormalTangent) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	//Create Index Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * indexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &indexBuffer);
		assert(SUCCEEDED(hr));
	}
}

void Mesh::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	lightViewVar->SetMatrix(v);
	lightProjVar->SetMatrix(p);
}

void Mesh::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	shadowTransformVar->SetMatrix(shadowTransform);

	if (srv != NULL)
		shadowMapVar->SetResource(srv);
}

void Mesh::ShadowUpdate()
{
}

void Mesh::NormalRender()
{
	Render();
}

void Mesh::ShadowRender(UINT tech, UINT pass)
{
	RenderShadow(tech, pass);
}