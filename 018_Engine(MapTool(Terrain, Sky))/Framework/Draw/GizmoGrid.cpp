#include "Framework.h"
#include "GizmoGrid.h"
#include "../Draw/LineMaker.h"

GizmoGrid::GizmoGrid(int gridSpacing)
	:spacing(gridSpacing)
{
	shader = new Shader(Effects + L"002_VertexColor.hlsl");
	gridSize = 512;
	lineColor = { 0,0,0.6f,1 };
	highlightColor = { 0,0,1.0f,1 };
	vertices = NULL;

	ResetLines();
}

GizmoGrid::~GizmoGrid()
{
	SAFE_DELETE(shader);
	SAFE_DELETE(vertices);
	SAFE_RELEASE(vertexBuffer);
}

void GizmoGrid::GridSpacing(int value)
{
	spacing = value;
	ResetLines();
}

void GizmoGrid::Render()
{
	UINT stride = sizeof(VertexColor);
	UINT offset = 0;


	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);

	shader->AsMatrix("World")->SetMatrix(world);

	shader->Draw(0,0, vertexCount);
}

void GizmoGrid::ResetLines()
{
	LineNums= ((gridSize / spacing) * 4) + 2 + 12;
	
	vector<VertexColor*> vertexArray;
	
	// fill array
	for (int i = 1; i < (gridSize / spacing) + 1; i++)
	{
		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)(i * spacing), 0, (float)gridSize), lineColor));
		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)(i * spacing), 0, -(float)gridSize), lineColor));

		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)(-i * spacing), 0, (float)gridSize), lineColor));
		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)(-i * spacing), 0, -(float)gridSize), lineColor));

		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)gridSize, 0, (float)(i * spacing)), lineColor));
		vertexArray.push_back(new VertexColor(D3DXVECTOR3(-(float)gridSize, 0, (float)(i * spacing)), lineColor));

		vertexArray.push_back(new VertexColor(D3DXVECTOR3((float)gridSize, 0, (float)(-i * spacing)), lineColor));
		vertexArray.push_back(new VertexColor(D3DXVECTOR3(-(float)gridSize, 0, (float)(-i * spacing)), lineColor));
	}

	// add highlights
	vertexArray.push_back(new VertexColor(D3DXVECTOR3(0,0,1) * (float)gridSize, highlightColor));
	vertexArray.push_back(new VertexColor(D3DXVECTOR3(0, 0, -1) * (float)gridSize, highlightColor));

	vertexArray.push_back(new VertexColor(D3DXVECTOR3(1, 0, 0) * (float)gridSize, highlightColor));
	vertexArray.push_back(new VertexColor(D3DXVECTOR3(-1, 0, 0) * (float)gridSize, highlightColor));

	vertexCount = vertexArray.size();

	if (vertices != NULL)
	{
		SAFE_DELETE(vertices);
		vertices = NULL;
	}
	vertices = new VertexColor[vertexCount];
	for (UINT i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = vertexArray[i]->Position;
		vertices[i].Color = vertexArray[i]->Color;
	}


	//Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexColor) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}
