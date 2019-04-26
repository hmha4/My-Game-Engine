#pragma once

class DeferredRendering
{
public:
	enum Target
	{
		Position = 0,
		Normal,
		Albedo,
		Specular,
		Shadow,
		Depth,

		Count
	};
public:
	DeferredRendering(float width, float height);
	~DeferredRendering();

	
	void Set();
	void Render();
	void ResizeScreen(float width, float height);

	Effect * GetEffect() { return effect->GetEffect(); }
	ID3D11ShaderResourceView *& ShadowSRV() { return srv[Shadow]; }
	ID3D11DepthStencilView * ShadowDSV() { return shadowDSV; }
	ID3D11ShaderResourceView *& DepthSRV() { return srv[Depth]; }
	ID3D11DepthStencilView * DepthDSV() { return shadowDSV; }

private:
	void CreateTextures();
	void CreateRTV();
	void CreateDSV(ID3D11Texture2D ** buffer, ID3D11DepthStencilView ** dsv);
	void CreateSRV();

private:
	float width;
	float height;

	Material * effect;

	ID3D11Texture2D * texture[4];
	ID3D11RenderTargetView * rtv[4];
	ID3D11ShaderResourceView * srv[6];

	ID3D11Texture2D * shadowBuffer;
	ID3D11DepthStencilView * shadowDSV;

	ID3D11Texture2D * depthBuffer;
	ID3D11DepthStencilView * depthDSV;

	Viewport * viewport;

private:
	Render2D * render[6];

public:
	static void Create(float width, float height);
	static void Delete();
	static DeferredRendering * Get();

private:
	static DeferredRendering * instance;
};