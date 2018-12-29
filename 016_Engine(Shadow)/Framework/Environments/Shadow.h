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
	void Delete();

public:
	void Add(class IShadow * shadow);

private:
	void BuildShadowTransform();

private:
	vector<IShadow *> objs;

	Shader * normalShader;
	Shader * shadowShader;

	UINT width;
	UINT height;

	DepthStencilView * depthMapDSV;
	RenderTargetView * depthMapRTV;

	Viewport * viewport;

	ID3D11ShaderResourceView * depthMapSRV;

	BSphere * sceneBounds;

	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;
	D3DXMATRIX shadowTransform;

	Render2D * depthShadow;
};