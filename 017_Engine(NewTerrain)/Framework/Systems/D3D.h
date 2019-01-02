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

	//	내가 직접 조작해서 윈도우 크기를 조정할 때
	void ResizeScreen(float width, float height);
	//	어플리케이션 내에서 윈도우 크기를 조정할 때
	void ResizeTarget(float width, float height);

	//	Device Lost 처리
	bool CheckDeviceLost();

	void SetToNull();
	
private:
	D3D();
	~D3D();

	void Init();
	void Release();

	//	그래픽 카드 선택
	void SetGpuInfo();
	void CreateSwapChainAndDevice();

	void CreateBackBuffer(float width, float height);
	void DeleteBackBuffer();

	//	필요없는 화면 렌더링 제어
	void StandBy();
private:
	static D3D					*instance;		//	D3D 객체
	static D3DDesc				d3dDesc;		//	D3DDesc
	static ID3D11Device			*device;		//	디바이스
	static ID3D11DeviceContext	*deviceContext;	//	DC : 그려주는 기능을 담당
	static IDXGISwapChain		*swapChain;		//	스왑체인 : 백버퍼를 가지고 있음

	ID3D11Debug					*debugDevice;

	wstring						gpuDescription;	//	GPU(그래픽카드) 이름
	UINT						gpuMemorySize;	//	GPU의 메모리 바이트 수
												//	(CPU와 공유 되고있지 않은)
	
	//화면 주사율 : numerator / denominator
	//	Vsync(true) -> 60 / 1
	//	Vsync(false) -> 0 / 1
	UINT						numerator;
	UINT						denominator;

	//	백 버퍼 : 그려주는 과정을 보이지 않기 위해
	//			  미리 백 버퍼에 그려주고 전면 버퍼에 시연
	ID3D11Texture2D				*backBuffer;		
	ID3D11DepthStencilView		*depthStencilView;	//	DSV
	ID3D11RenderTargetView		*renderTargetView;	//	RTV
};