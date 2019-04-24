#pragma once

class GizmoGrid
{
public:
	GizmoGrid(int gridSpacing = 8);
	~GizmoGrid();

	void Initialize();
	void Ready();

	void GridSpacing(int value);
	void Render();
private:
	void ResetLines();

private:
	int spacing;
	int gridSize;
	int LineNums;
	UINT vertexCount;

	Effect*effect;
	ID3D11Buffer* vertexBuffer;

	D3DXCOLOR lineColor;
	D3DXCOLOR highlightColor;

	VertexColor*vertices;

private:
	ID3DX11EffectMatrixVariable * worldVariable;
};