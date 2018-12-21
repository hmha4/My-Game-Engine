#pragma once

class Grass
{
private:
	enum class LevelOfDetail
	{
		LEVEL1 = 0,
		LEVEL2 = 50,
		LEVEL3 = 60,
		LEVEL4 = 70
	};

public:
	Grass(UINT terrainSize, class Terrain * terrain);
	~Grass();

	void Update();
	void Render();

private:
	void InitWind();

	void GenerateGrass(UINT numberOfPatchRows = 50, UINT numberOfRootsInPatch = 70);
	int GeneratePatch(D3DXVECTOR3 startPos, D3DXVECTOR3 patchSize, int currentVertex);
	void DrawGrass();

	void GenerateFlower();
	void DrawFlower();
private:
	Texture * grassBlade;
	Texture * grassBladeAlpha1;
	Texture * grassBladeAlpha2;

	Texture * flower;

	Shader * shader;

	VertexTextureNormal * vertices;
	ID3D11Buffer * vertexBuffer;

	ID3D11Buffer * vertexBufferF;

	UINT numOfPatchRows;
	UINT numOfRoots;
	UINT numOfPatches;
	UINT numOfRootsInPatch;

	UINT terrainSize;

	class Terrain * terrain;

private:
	class Wind
	{
	public:
		Wind() {}
		Wind(D3DXVECTOR2 accel, D3DXVECTOR2 velocity)
			: accel(accel), velocity(velocity) 
		{}
		~Wind() {}

		//void Accel(D3DXVECTOR2 val) { accel = val; }
		D3DXVECTOR2& Accel() { return accel; }

		//void Velocity(D3DXVECTOR2 val) { velocity = val; }
		D3DXVECTOR2& Velocity() { return velocity; }
	private:
		D3DXVECTOR2 accel;
		D3DXVECTOR2 velocity;
	};

	Wind * windField;
};