#pragma once

class Shadow
{
public:
	Shadow(UINT width, UINT height);
	~Shadow();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();

public:
	void Add(class IShadow * shadow);

private:
	void BuildShadowTransform();

private:
	bool isShadow;
	vector<IShadow *> objs;

	UINT width;
	UINT height;

	Viewport * viewport;

	DepthStencilView * depthMapDSV;
	ID3D11ShaderResourceView * depthMapSRV;

	BSphere * sceneBounds;

	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;
	D3DXMATRIX shadowTransform;

	Render2D * depthShadow;
};