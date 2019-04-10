#pragma once

//	TODO : Grass Material 수정
class Flower : public IShadow
{
	struct FlowerDesc;
	class Wind;

public:
	Flower(class Terrain * terrain);
	~Flower();

	void Initialize();
	void Ready();
	void Update();
	void Render();
	void RenderShader(UINT tech);
	void ImGuiRender();

	void Delete();
	void SetVelocity();
	void CreateVector();
	void GenerateFlower(D3DXVECTOR3 position, float size, UINT numOfRoot);
	void GenerateFlower(FlowerDesc * desc);

	vector<FlowerDesc *>& Flowers() { return flowers; }
	FlowerDesc * GetDesc() { return flowerDesc; }
	UINT& FlowerCount() { return flowerCount; }
	Wind * GetWind() { return windField; }
	Material * GetMaterial() { return material; }

private:
	//	Wind
	void InitWind();

	//	Load
	void LoadData();
	void BindData();

	void LoadFlowerMapFile(wstring filename);

	//	Draw Grass & Flower
	void DrawFlower(UINT tech = 0, UINT pass = 0);
private:
	class Terrain * terrain;
	Material * material;

	UINT flowerCount;

	class Frustum * frustum;
	D3DXPLANE frustumPlane[6];
	// ====================================================== //
	//	Grass
	// ====================================================== //
	vector<FlowerDesc *> flowers;

	vector<VertexTextureNormal> vertices;
	UINT flowerRootsNum;
	UINT flowerRootsNumBefore;

	ID3D11Buffer * vertexBuffer;

private:
	ID3DX11EffectVectorVariable * frustumPlaneVar;

	ID3DX11EffectVectorVariable * locationVar;
	ID3DX11EffectVectorVariable * colorVar;
	ID3DX11EffectScalarVariable * rangeVar;

	ID3DX11EffectMatrixVariable * lightViewVar;
	ID3DX11EffectMatrixVariable * lightProjVar;
	ID3DX11EffectMatrixVariable * shadowTransformVar;
	ID3DX11EffectShaderResourceVariable * shadowMapVar;

private:
	struct FlowerDesc
	{
		Material * material;
		wstring DiffuseMap;

		UINT FlowerNum;
		float Size;

		D3DXVECTOR3 Position;

		UINT RootsNum;

		FlowerDesc() {}
		FlowerDesc(Material * material)
		{
			this->material = material;

			Size = 0;
			Position = D3DXVECTOR3(0, 0, 0);
			FlowerNum = 0;
			RootsNum = 0;
			DiffuseMap = material->GetDiffuseMap()->GetFile();
		}

		void ChangeTexture(wstring fileName)
		{
			this->material->SetDiffuseMap(fileName);
		}

		void Delete()
		{
		}
	} *flowerDesc;

private:
	class Wind
	{
	public:
		Wind() {}
		Wind(D3DXVECTOR2 accel, D3DXVECTOR2 velocity)
			: accel(accel), velocity(velocity)
		{}
		~Wind() {}

		D3DXVECTOR2& Accel() { return accel; }
		D3DXVECTOR2& Velocity() { return velocity; }
	private:
		D3DXVECTOR2 accel;
		D3DXVECTOR2 velocity;
	};

	Wind * windField;

	// IShadow을(를) 통해 상속됨
	virtual void SetEffect(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	virtual void ShadowUpdate() override;
	virtual void NormalRender() override;
	virtual void ShadowRender(UINT tech, UINT pass) override;
};