#pragma once

class Rain
{
private:
	struct VertexRain;

public:
	Rain(D3DXVECTOR3& size, UINT count);
	~Rain();

	void Update();
	void Render();

private:
	Effect * effect;

	ID3D11Buffer	*vertexBuffer;

	UINT			particleCount;

	TextureArray	*rainMaps;

private:
	struct VertexRain
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR2 Scale;
		UINT TextureId;
		float Random;

		VertexRain() : Position(0, 0, 0), Scale(0, 0), TextureId(0), Random(0.0f) {}
		VertexRain(D3DXVECTOR3& position, D3DXVECTOR2& scale, UINT& textureId, float& random)
			: Position(position), Scale(scale), TextureId(textureId), Random(random) {}
	};
};