#pragma once

class Terrain
{
public:
	Terrain();
	~Terrain();

	void Update();
	void Render();

	UINT Width() { return width; }
	UINT Height() { return height; }

	Texture* HeightMap() { return heightMap; }

	float GetHeight(float positionX, float positionZ);

private:
	Texture* heightMap;

	Shader	* shader;
	Texture * diffuseMap;

	ID3D11Buffer* vertexBuffer, *indexBuffer;
	VertexTextureNormal* vertices;
	UINT* indices;

	UINT width, height;
	UINT vertexCount, indexCount;
};