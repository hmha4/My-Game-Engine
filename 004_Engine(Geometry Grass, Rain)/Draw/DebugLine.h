#pragma once

class DebugLine
{
public:
	DebugLine();
	~DebugLine();

	void Color(float r, float g, float b);
	void Color(D3DXCOLOR& vec);

	void Draw(D3DXMATRIX& world, class ILine* line);
	void Render();

private:
	static const UINT MaxCount;

private:
	Shader* shader;
	ID3D11Buffer* vertexBuffer;

	D3DXCOLOR color;
	struct Line
	{
		D3DXVECTOR3 start;
		D3DXVECTOR3 end;
	};
	vector<Line> lines;
};