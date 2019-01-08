#pragma once

class GizmoGrid
{
public:
	GizmoGrid(int gridSpacing = 8);
	~GizmoGrid();
	void GridSpacing(int value);
	void Render();
private:
	void ResetLines();

private:
	int spacing;
	int gridSize;
	int LineNums;
	UINT vertexCount;

	Shader*shader;
	ID3D11Buffer* vertexBuffer;

	D3DXCOLOR lineColor;
	D3DXCOLOR highlightColor;

	VertexColor*vertices;
	class LineMaker*lines[8];
};