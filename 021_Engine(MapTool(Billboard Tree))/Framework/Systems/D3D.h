#pragma once

struct D3DDesc
{
	wstring		AppName;
	HINSTANCE	Instance;
	HWND		Handle;
	float		Width;
	float		Height;
	bool		bVsync;
	bool		bFullScreen;
	bool		bStandBy;
};

class D3D
{
public:
	static D3D *Get() { return instance; }

	static void Create();
	static void Delete();

	static ID3D11Device *GetDevice() { return device; }
	static ID3D11DeviceContext *GetDC() { return deviceContext; }
	static IDXGISwapChain *GetSwapChain() { return swapChain; }

	static void GetDesc(D3DDesc* desc) { *desc = d3dDesc; }
	static void SetDesc(D3DDesc& desc) { d3dDesc = desc; }

	void SetRenderTarget(ID3D11RenderTargetView *rtv = NULL, ID3D11DepthStencilView *dsv = NULL);
	void SetRenderTargets(UINT count, ID3D11RenderTargetView **rtv, ID3D11DepthStencilView *dsv = NULL);

	ID3D11RenderTargetView* GetRenderTargetView() { return renderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView() { return depthStencilView; }

	void Clear(D3DXCOLOR color = D3DXCOLOR(0xFF6a6a80), ID3D11RenderTargetView *rtv = NULL, ID3D11DepthStencilView *dsv = NULL);
	void Present();

	//	���� ���� �����ؼ� ������ ũ�⸦ ������ ��
	void ResizeScreen(float width, float height);
	//	���ø����̼� ������ ������ ũ�⸦ ������ ��
	void ResizeTarget(float width, float height);

	//	Device Lost ó��
	bool CheckDeviceLost();

	void SetToNull();
	
private:
	D3D();
	~D3D();

	void Init();
	void Release();

	//	�׷��� ī�� ����
	void SetGpuInfo();
	void CreateSwapChainAndDevice();

	void CreateBackBuffer(float width, float height);
	void DeleteBackBuffer();

	//	�ʿ���� ȭ�� ������ ����
	void StandBy();
private:
	static D3D					*instance;		//	D3D ��ü
	static D3DDesc				d3dDesc;		//	D3DDesc
	static ID3D11Device			*device;		//	����̽�
	static ID3D11DeviceContext	*deviceContext;	//	DC : �׷��ִ� ����� ���
	static IDXGISwapChain		*swapChain;		//	����ü�� : ����۸� ������ ����

	ID3D11Debug					*debugDevice;

	wstring						gpuDescription;	//	GPU(�׷���ī��) �̸�
	UINT						gpuMemorySize;	//	GPU�� �޸� ����Ʈ ��
												//	(CPU�� ���� �ǰ����� ����)
	
	//ȭ�� �ֻ��� : numerator / denominator
	//	Vsync(true) -> 60 / 1
	//	Vsync(false) -> 0 / 1
	UINT						numerator;
	UINT						denominator;

	//	�� ���� : �׷��ִ� ������ ������ �ʱ� ����
	//			  �̸� �� ���ۿ� �׷��ְ� ���� ���ۿ� �ÿ�
	ID3D11Texture2D				*backBuffer;		
	ID3D11DepthStencilView		*depthStencilView;	//	DSV
	ID3D11RenderTargetView		*renderTargetView;	//	RTV
};