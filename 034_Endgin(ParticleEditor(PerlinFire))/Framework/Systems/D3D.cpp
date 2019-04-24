#include "Framework.h"
#include "D3D.h"

D3D * D3D::instance = NULL;
D3DDesc D3D::d3dDesc;
ID3D11Device * D3D::device = NULL;
ID3D11DeviceContext * D3D::deviceContext = NULL;
IDXGISwapChain * D3D::swapChain = NULL;

void D3D::Create()
{
	assert(instance == NULL);

	instance = new D3D();
}

void D3D::Delete()
{
	SAFE_DELETE(instance);
}

void D3D::SetRenderTarget(ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv)
{
	if (rtv == NULL)
		rtv = renderTargetView;
	if (dsv == NULL)
		dsv = depthStencilView;

	//	RTV�� DSV�� OM(Output Merger)
	//	���������� ���� Ÿ������ ����
	//	DX11���� ���ÿ� �ִ� 8�� ���� ���� ����
	//	���� Ÿ���� 8�� ���� DSV�� �ϳ��̴�.
	deviceContext->OMSetRenderTargets(1, &rtv, dsv);
}

void D3D::SetRenderTargets(UINT count, ID3D11RenderTargetView ** rtv, ID3D11DepthStencilView * dsv)
{
	//	Multi Render Target
	deviceContext->OMSetRenderTargets(count, rtv, dsv);
}

void D3D::Clear(D3DXCOLOR color, ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv)
{
	if (rtv == NULL)
		rtv = renderTargetView;
	if (dsv == NULL)
		dsv = depthStencilView;

	//	�� ���� Clear
	//	rtv : clear�� ���� Ÿ�� ��
	//	color : �ٲ� ���� ��
	deviceContext->ClearRenderTargetView(rtv, color);

	//	����/���ٽ� ���� Clear
	//	dsv : clear�� ����/���ٽ� ��
	//	D3D11_CLEAR_DEPTH : ���� ���� Ŭ���� �ϰڴ�.
	//	D3D11_CLEAR_STENCIL : ���ٽ� ���� Ŭ���� �ϰڴ�.
	//	1 : �ʱ�ȭ �� ���� ��
	//	0 : �ʱ�ȭ �� ���ٽ� ��
	deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1, 0);
}

void D3D::Present()
{
	//swapChain->Present(d3dDesc.bVsync == true ? 1 : 0, 0);
	StandBy();
}

void D3D::ResizeScreen(float width, float height)
{
	if (width < 1 || height < 1)
		return;

	d3dDesc.Width = width;
	d3dDesc.Height = height;

	DeleteBackBuffer();
	{
		HRESULT hr = swapChain->ResizeBuffers
		(
			0,							//	�� ������ ��
			(UINT)width, (UINT)height,	//	ȭ�� ����, ����
			DXGI_FORMAT_R8G8B8A8_UNORM,	//	����� �ؽ��� ����
			0							//	������ DXGI_SWAP_CHAIN_FLAG
		);

		assert(SUCCEEDED(hr));
	}
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

///////////////////////////////////////////////////////////////////////////////
//	ResizeTarget
//	- ������, Ǯ ��ũ�� ��忡 ������� �����Ѵ�.
//	- WM_SIZE�޼����� �߻��Ѵ�.
//	- WM_SIZE �޼����� �߻����� �� ����� ���Ÿ� ���ָ� �ȴ�.
///////////////////////////////////////////////////////////////////////////////
void D3D::ResizeTarget(float width, float height)
{
	if (width < 1 || height < 1)
		return;

	d3dDesc.Width = width;
	d3dDesc.Height = height;

	DXGI_MODE_DESC desc;
	desc.Width = (UINT)d3dDesc.Width;
	desc.Height = (UINT)d3dDesc.Height;
	if (d3dDesc.bVsync == true)
	{
		desc.RefreshRate.Numerator = numerator;
		desc.RefreshRate.Denominator = 1;
	}
	else
	{
		desc.RefreshRate.Numerator = 0;
		desc.RefreshRate.Denominator = 1;
	}
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	HRESULT hr = swapChain->ResizeTarget(&desc);
	assert(SUCCEEDED(hr));
}

bool D3D::CheckDeviceLost()
{
	///////////////////////////////////////////////////////////////////////////
	//	DirectX 11������ ����̽� �ν�Ʈ�� ���� �Ͼ�� ������,
	//	��� ������ ��Ȳ������ ����̽� �ν�Ʈ�� �Ͼ��.
	//	��) GPU�� Hang Up���°� �Ǿ� ��ð� ������ ���� ���
	//		OS�� Ÿ�� �ƿ�ó���� �ߵ��Ͽ� GPU�� ���� �ȴ�.
	//		�� ��쿡�� OS�� ������� �ʿ������ ���ø����̼�������
	//		���� DirectX 11����̽��� ������Ʈ���� ��� �ı��ϰ� ���� �������Ѵ�.
	//	����̽� �ν�Ʈ�� �Ͼ������ ID3D11Device::GetDeviceRemovedReason�Լ��� ȣ���Ͽ� ���� �� �� �ִ�.
	//	��ȯ���� S_OK�� �ƴ� ��� ��� ������ ���� ����̽� �ν�Ʈ�� �߻��Ѵ�.
	///////////////////////////////////////////////////////////////////////////

	HRESULT hr;

	hr = device->GetDeviceRemovedReason();	//	����̽� �ν�Ʈ�� �Ͼ���� ����
	assert(SUCCEEDED(hr));

	switch (hr)
	{
	case S_OK: break;	//	����
	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		Release();		//	Device Release
		Init();			//	Device Initialize

		if (FAILED(hr))
			return false;//	���� ���ø����̼� ����

		break;
	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		return false;	//	���� ���ø����̼� ����
	}

	return true;
}

void D3D::SetToNull()
{
	ID3D11RenderTargetView* nullRTV = NULL;
	D3D::GetDC()->OMSetRenderTargets(1, &nullRTV, NULL);

	ID3D11ShaderResourceView * null[1] = { 0 };
	D3D::GetDC()->PSSetShaderResources(0, 1, null);
}

void D3D::StandBy()
{
	HRESULT hr;
	///////////////////////////////////////////////////////////////////////////////
	//	DXGI_STATUS_OCCLUDED
	//	- ȭ���� �ּ�ȭ �ǰų� �Ⱥ��̸�
	//	- DXGI_STATUS_OCCLUDED�� ��ȯ�Ѵ�.
	///////////////////////////////////////////////////////////////////////////////
	if (d3dDesc.bStandBy == true)
	{
		hr = swapChain->Present(d3dDesc.bVsync == true ? 1 : 0, DXGI_PRESENT_TEST);

		if (hr == DXGI_STATUS_OCCLUDED)
			return;

		d3dDesc.bStandBy = false;
	}

	hr = swapChain->Present(d3dDesc.bVsync == true ? 1 : 0, 0);

	if (hr == DXGI_STATUS_OCCLUDED)
		d3dDesc.bStandBy = true;
}

D3D::D3D()
	: numerator(0), denominator(1)
{
	//	�׷��� ī�� ���� ����
	SetGpuInfo();

	//	����ü�ΰ� ����̽��� ���� ����
	//	DC�� ���� ����
	CreateSwapChainAndDevice();
	//	����ü���� �� ���� ����
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

D3D::~D3D()
{
	DeleteBackBuffer();

	//	������ ������ ��� ������
	//	�ʱ�ȭ ��Ų��.
	if (deviceContext != NULL)
	{
		deviceContext->ClearState();
		deviceContext->Flush();
	}

	if (swapChain != NULL)
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(swapChain);

#ifdef _DEBUG
	ID3D11Debug* DebugDevice = nullptr;
	HRESULT hr = device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&DebugDevice));
	assert(SUCCEEDED(hr));

	hr = DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(DebugDevice);
#endif

	SAFE_RELEASE(device);
}

void D3D::Init()
{
	numerator = 0;
	denominator = 1;

	//	�׷��� ī�� ���� ����
	SetGpuInfo();

	//	����ü�ΰ� ����̽��� ���� ����
	//	DC�� ���� ����
	CreateSwapChainAndDevice();
	//	����ü���� �� ���� ����
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

void D3D::Release()
{
	DeleteBackBuffer();

	//	������ ������ ��� ������
	//	�ʱ�ȭ ��Ų��.
	if (deviceContext != NULL)
		deviceContext->ClearState();

	if (swapChain != NULL)
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
}

void D3D::SetGpuInfo()
{
	///////////////////////////////////////////////////////////////////////////////
	//	DXGI : DirectX Graphics Infrastructure
	//		   ���̷�ƮX �׷��Ƚ� �Ϻα���
	//	- DirectX 11���� �׷��Ƚ� ����̳�
	//	- ���ø����̼����κ��� ���� ǥ�ø� �޾�
	//	- Ŀ�θ�� ����̹��� �ϵ����� �ְ� �޴� ����
	///////////////////////////////////////////////////////////////////////////////

	HRESULT hr;
	IDXGIFactory *factory = NULL;
	IDXGIAdapter *adapter = NULL;
	IDXGIOutput *output = NULL;
	DXGI_MODE_DESC *displayModeList = NULL;
	//	1. DXGI�� ����� ����Ϸ��� �켱 IDXGIFactory�������̽��� ���;��Ѵ�.
	{
		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		assert(SUCCEEDED(hr));
	}
	//	2. �׷��� ī�� ���� ���
	{
		//	�׷��� ī�� ���� ����

		hr = factory->EnumAdapters(0, &adapter);
		assert(SUCCEEDED(hr));

		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);

		wprintf_s(L"-------------------------------\n\n");
		wprintf_s(L"<Graphics Card Info>\n\n");
		wprintf_s(L"Graphics Card : %s\n", adapterDesc.Description);
		wprintf_s(L"Vendroid : %d\n", adapterDesc.VendorId);
		wprintf_s(L"DeviceID : %d\n", adapterDesc.DeviceId);
		wprintf_s(L"SubSysID : %d\n", adapterDesc.SubSysId);
		wprintf_s(L"Revision : %d\n", adapterDesc.Revision);
		//	CPU�� �����ǰ� ���� ���� ���� �޸� ����Ʈ ��
		wprintf_s(L"DedicatedVideoMemory : %dMB\n", adapterDesc.DedicatedVideoMemory / 1024 / 1024);
		//	GPU�� �����ǰ� ���� ���� �ý��� �޸� ����Ʈ ��
		wprintf_s(L"DedicatedSystemMemory : %dMB\n", adapterDesc.DedicatedSystemMemory);
		//	�����ǰ� �ִ� �ý��� �޸� ����Ʈ ��
		wprintf_s(L"SharedSystemMemory : %dMB\n\n", adapterDesc.SharedSystemMemory / 1024 / 1024);
		wprintf_s(L"-------------------------------\n\n");

		//	���÷��� ���� ����
		hr = adapter->EnumOutputs(0, &output);
		assert(SUCCEEDED(hr));

		DXGI_OUTPUT_DESC outputDesc;
		output->GetDesc(&outputDesc);

		wprintf_s(L"<Display Info>\n\n");
		wprintf_s(L"Device Name : %s\n", outputDesc.DeviceName);
		//	����ũž ��ǥ���� ��� ����
		wprintf_s(L"DesktopCoordLeftTop : %d, %d\n", outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.top);
		wprintf_s(L"DesktopCoordRightBottom : %d, %d\n", outputDesc.DesktopCoordinates.right, outputDesc.DesktopCoordinates.bottom);
		//	����� ����ũž�� �����Ǿ� �ִ°�
		wprintf_s(L"IsAttackedToDesktop : %s\n", outputDesc.AttachedToDesktop == 0 ? L"False" : L"True");

		//	�� ��忡 �����ϴ� GPU ���� ������
		//	Display Mode�� ������ ����
		UINT modeCount;
		hr = output->GetDisplayModeList
		(
			DXGI_FORMAT_R8G8B8A8_UNORM
			, DXGI_ENUM_MODES_INTERLACED
			, &modeCount
			, NULL
		);
		assert(SUCCEEDED(hr));

		displayModeList = new DXGI_MODE_DESC[modeCount];
		//	displayModeList �ʱ�ȭ
		hr = output->GetDisplayModeList
		(
			DXGI_FORMAT_R8G8B8A8_UNORM
			, DXGI_ENUM_MODES_INTERLACED
			, &modeCount
			, displayModeList
		);
		assert(SUCCEEDED(hr));

		for (UINT i = 0; i < modeCount; i++)
		{
			bool isCheck = true;
			isCheck &= displayModeList[i].Width == d3dDesc.Width;
			isCheck &= displayModeList[i].Height == d3dDesc.Height;

			if (isCheck == true)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// ALT + ENTER ȭ�� ��� ��ȯ
	hr = factory->MakeWindowAssociation(d3dDesc.Handle, 0);
	assert(SUCCEEDED(hr));

	//	Release
	SAFE_DELETE_ARRAY(displayModeList);
	SAFE_RELEASE(output);
	SAFE_RELEASE(adapter);
	SAFE_RELEASE(factory);
}

void D3D::CreateSwapChainAndDevice()
{
	//	1. ����� ���� ���� ����
	//	���� �Ǵ� DX����
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	//	2. ����̽��� ����ü�� ����
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	//	D3D11_CREATE_DEVICE_SINGLETHREADED	: �̱� ������ ���� ����̽�
	//	D3D11_CREATE_DEVICE_DEBUG			: ����� ���̾�
	//	D3D11_CREATE_DEVICE_SWITCH_TO_REF	: ���۷��� Ÿ������ ��ü �Ҽ� �ִ� ����̽�
	//	D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS : ������ �����尡 ��������� �ʵ��� �Ѵ�. �Ϲ������� ������� ����
	//	D3D11_CREATE_DEVICE_BGRA_SUPPORT	: Direct2D�� Direct3D ���ҽ��� ��ȣ��� �����ϰ� �Ѵ�.
#if defined(_DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//	����� ���̾ ��� �����ϰ� �صθ�
	//	���α׷��� ������ �������� ������
	//	������ ��忡���� ������� �ʴ´�.
#endif

	//	3. ����ü�� ����
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;				//	�� ������ ����
	swapChainDesc.BufferDesc.Width = 0;			//	�� ������ ����, ����
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//	8bit, 0.0 ~ 1.0
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//	ȭ�� �ֻ���
	if (d3dDesc.bVsync == true)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//	�� ���۸� � �뵵�� ����� ������
	swapChainDesc.OutputWindow = d3dDesc.Handle;					//	�� ���۸� ����� ������ �ڵ�
	swapChainDesc.SampleDesc.Count = 1;								//	��Ƽ ���ø� ��
	swapChainDesc.SampleDesc.Quality = 0;							//	��Ƽ ���ø� ����Ƽ
	swapChainDesc.Windowed = !d3dDesc.bFullScreen;					//	��������
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//	DX�� �̿��� ��� ���α׷���
	//	Device�� �����ϴ� ���� ���� �⺻�̴�.
	//	Device�� SwapChain�� �� �Լ��� ���� ���ÿ� �����Ѵ�.
	//	�̶� ���� �������� ���� �� ������ DeviceContext�� ���� �����ȴ�.
	//	DX11�� ���� �ٽ��� �Լ��̴�.
	HRESULT hr = D3D11CreateDeviceAndSwapChain
	(
		NULL							//	����� IDXGIAdapter �������̽�
										//	�׷��� ī�带 ������ �ʿ䰡 ������ NULL�� �Ѵ�.
										//	NULL�� �ϸ� ���ʷ� �߰ߵ� �׷��� ī�尡 �����ȴ�.
		//D3D_DRIVER_TYPE_HARDWARE		:	�ϵ���� ����̺� ���(DX9->HAL), Direct3D�� �������� �κ��� �������� �ʰ�
		//D3D_DRIVER_TYPE_WARP			:	��� ���۸ս��� ����Ʈ���� �����Ͷ�����
		// ==> �׷��� ī�尡 ���ķ����� �������� �ʴ� ��������
		//		WARP ����̽��� �����ϸ� ���ķ��� 9_1 ~ 10_1���� �����Ѵ�.
		//		������ ��Ÿ �̻󿡼���, �׸��� DX11�̻󿡼���
		//D3D_DRIVER_TYPE_REFERENCE		:	���۷��� �����Ͷ�����(DX9->REF), �׽�Ʈ�� �������θ� ���
		//	==> ��� ���ķ����� �����Ѵ�.
		//		������ SDK�� ��ġ�Ǿ��ִ� ȯ�濡���� �����Ѵ�.
		//D3D_DRIVER_TYPE_SOFTWARE		:	���ุ �Ǿ��ְ�, ������� ����
		//D3D_DRIVER_TYPE_NULL			:	�������� �Ұ����� ���۷��� ����̹�
		//D3D_DRIVER_TYPE_UNKNOWN		:	���� �Ҹ�
		, D3D_DRIVER_TYPE_HARDWARE		//	DX11 ����̽� ����
		, NULL							//
		, creationFlags					//	����̽� �÷���(API ���̾�)
		, featureLevels					//	��� ������ DX����
		, 1								//	���� ���� �迭 ����
		, D3D11_SDK_VERSION				//	SDK ����
		, &swapChainDesc				//	����ü�� ����ü
		, &swapChain					//	�Ѱ� ���� ����ü�� �������̽�
		, &device						//	�Ѱ� ���� ����̽� �������̽�
		, NULL							//	���� ������ �� ������
		, &deviceContext				//	�Ѱ� ���� ����̽� ���ؽ�Ʈ(DC)
	);
	assert(SUCCEEDED(hr));	//	�׷��� ī�尡 �� DX������ �������� ���� ��
}

void D3D::CreateBackBuffer(float width, float height)
{
	//	����̽�, DC, ����ü���� �����߾
	//	����ü���� ����۰� ���� Ÿ������ ���� �Ǿ����� �ʴ�.
	//	�׷��� ����ü�����κ��� ����۸� ���ͼ�
	//	����̽��� ���� Ÿ������ �����ؾ��Ѵ�.
	HRESULT hr;

	//	���� ü���� ����۸� D3DX ����Ÿ�ٿ� ����
	{
		//	����ü�����κ��� �� ���۸� ����
		//	�� ���۴� �⺻������ 2D�ؽ��ķ� ������� �ֱ� ������
		//	ID3D11Texture2D�� �� ���۸� �޾ƿ´�.
		ID3D11Texture2D *pBackBuffer = NULL;
		hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		assert(SUCCEEDED(hr));

		//	�ؽ��Ĵ� �������������� ���� �並 ���� ������ ������
		//	���� Ÿ�ٵ� �ؽ����� �����̱� ������ �並 �̿��� �����ؾ��Ѵ�.

		//	���� Ÿ�� �� ����
		hr = device->CreateRenderTargetView
		(
			pBackBuffer,		//	�信�� ������ �ϴ� ������
			NULL,				//	���� Ÿ�� �� ����
			&renderTargetView	//	��ȯ�� RTV�� �޾ƿ� ������ ������
		);
		assert(SUCCEEDED(hr));

		//	���� Ÿ�� �䰡 �����ǰ� ����
		//	���� ������� �����Ϳ� �������� �ʰ�
		//	�� ���� Ÿ�� �並 ����Ѵ�.
		//	�׷��� ������ �ش�.
		SAFE_RELEASE(pBackBuffer);
	}

	// ����/���ٽ� ���� ����
	//	: DXD11�� ���� ���ۿ� ���ٽ� ���۴� 1���� ���ҽ��� �����Ѵ�.
	//	: DxD11������ ����/���ٽ� ���۴� �ؽ��� ���ҽ��� �� �����̴�.
	{
		//	����/���ٽ� �ؽ��� ����
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		//	�Ӹ� ���� : Anti-Aliasing ��� �� �ϳ��̴�.
		//	�ؽ����� LOD���� �����̴�.
		//	�ؽ��ĸ� �ϳ� ����� ���������� ���� ũ��
		//	�ܰ��� �ؽ��ĸ� ����� �δ� ��
		//	��Ȳ�� �°� �´� ũ���� �ؽ��ĸ� ���
		desc.MipLevels = 1;
		desc.ArraySize = 1;								//	�迭 ������
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//	���� Depth(24bit), Stencil(8bit)
		desc.SampleDesc.Count = 1;						//	��Ƽ ���ø� ��
		desc.SampleDesc.Quality = 0;					//	��Ƽ ���ø� ����Ƽ
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		//	����/���ٽ� ���۷� ����ϰڴ�.
		desc.CPUAccessFlags = 0;						//	CPU�� ���ʹ� �������� �ʰڴ�.
		desc.MiscFlags = 0;								//	�� �� ���� ����

														//	�ؽ��� ����
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &backBuffer);
		assert(SUCCEEDED(hr));
	}

	//	�ؽ��Ĵ� �ؽ��� ���ҽ��� �� ���� �̹Ƿ�
	//	���������ο� �����ϱ� ���ؼ��� �並 ����Ѵ�.
	//	����/���ٽ� ��(Depth_Stencil_View)�� ����
	//
	{
		//	�� ���ۿ� �׻� ���� ����
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		//	�ؽ��� ������ ����� ���� �״�� ���
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//	2D �ؽ��� ���
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateDepthStencilView(backBuffer, &desc, &depthStencilView);
		assert(SUCCEEDED(hr));

		//	���� Ÿ�� �� ����
		SetRenderTarget(renderTargetView, depthStencilView);
	}
}

void D3D::DeleteBackBuffer()
{
	SAFE_RELEASE(depthStencilView);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(backBuffer);
}