#pragma once

class DebugLine
{
public:
	DebugLine(Effect * effect);
	~DebugLine();

	void Initialize();
	void Ready();

	void Color(float r, float g, float b);
	void Color(D3DXCOLOR& vec);

	void Draw(D3DXMATRIX& world, class ILine* line);
	void Render(UINT pass = 0, D3DXMATRIX *world = NULL);

private:
	static const UINT MaxCount;

private:
	Effect * effect;
	ID3D11Buffer* vertexBuffer;

	D3DXCOLOR color;
	struct Line
	{
		D3DXVECTOR3 start;
		D3DXVECTOR3 end;
	};
	vector<Line> lines;

private:
	ID3DX11EffectMatrixVariable * worldVar;
	ID3DX11EffectVectorVariable * lineColorVar;
};