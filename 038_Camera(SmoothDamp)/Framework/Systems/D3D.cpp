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

	//	RTV와 DSV를 OM(Output Merger)
	//	스테이지의 렌더 타겟으로 설정
	//	DX11에서 동시에 최대 8개 까지 설정 가능
	//	렌더 타겟이 8개 여도 DSV는 하나이다.
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

	//	백 버퍼 Clear
	//	rtv : clear할 렌더 타겟 뷰
	//	color : 바꿀 색상 값
	deviceContext->ClearRenderTargetView(rtv, color);

	//	깊이/스텐실 버퍼 Clear
	//	dsv : clear할 깊이/스텐실 뷰
	//	D3D11_CLEAR_DEPTH : 깊이 값을 클리어 하겠다.
	//	D3D11_CLEAR_STENCIL : 스텐실 값을 클리어 하겠다.
	//	1 : 초기화 할 깊이 값
	//	0 : 초기화 할 스텐실 값
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
			0,							//	백 버퍼의 수
			(UINT)width, (UINT)height,	//	화면 넓이, 높이
			DXGI_FORMAT_R8G8B8A8_UNORM,	//	백버퍼 텍스쳐 포맷
			0							//	조합할 DXGI_SWAP_CHAIN_FLAG
		);

		assert(SUCCEEDED(hr));
	}
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

///////////////////////////////////////////////////////////////////////////////
//	ResizeTarget
//	- 윈도우, 풀 스크린 모드에 상관없이 동작한다.
//	- WM_SIZE메세지를 발생한다.
//	- WM_SIZE 메세지가 발생했을 때 백버퍼 갱신만 해주면 된다.
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
	//	DirectX 11에서는 디바이스 로스트가 거의 일어나지 않지만,
	//	몇가지 예외적 상황에서는 디바이스 로스트가 일어난다.
	//	예) GPU가 Hang Up상태가 되어 장시간 응답이 없을 경우
	//		OS의 타임 아웃처리가 발동하여 GPU가 리셋 된다.
	//		이 경우에도 OS의 재부팅은 필요없지만 어플리케이션측에서
	//		만든 DirectX 11디바이스나 오브젝트등은 모두 파기하고 새로 만들어야한다.
	//	디바이스 로스트가 일어났는지는 ID3D11Device::GetDeviceRemovedReason함수를 호출하여 조사 할 수 있다.
	//	반환값이 S_OK가 아닌 경우 몇가지 이유로 인해 디바이스 로스트가 발생한다.
	///////////////////////////////////////////////////////////////////////////

	HRESULT hr;

	hr = device->GetDeviceRemovedReason();	//	디바이스 로스트가 일어났는지 조사
	assert(SUCCEEDED(hr));

	switch (hr)
	{
	case S_OK: break;	//	정상
	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		Release();		//	Device Release
		Init();			//	Device Initialize

		if (FAILED(hr))
			return false;//	실패 어플리케이션 종료

		break;
	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		return false;	//	실패 어플리케이션 종료
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
	//	- 화면이 최소화 되거나 안보이면
	//	- DXGI_STATUS_OCCLUDED를 반환한다.
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
	//	그래픽 카드 정보 세팅
	SetGpuInfo();

	//	스왑체인과 디바이스를 같이 생성
	//	DC도 같이 생성
	CreateSwapChainAndDevice();
	//	스왑체인의 백 버퍼 설정
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

D3D::~D3D()
{
	DeleteBackBuffer();

	//	파이프 라인의 모든 설정을
	//	초기화 시킨다.
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

	//	그래픽 카드 정보 세팅
	SetGpuInfo();

	//	스왑체인과 디바이스를 같이 생성
	//	DC도 같이 생성
	CreateSwapChainAndDevice();
	//	스왑체인의 백 버퍼 설정
	CreateBackBuffer(d3dDesc.Width, d3dDesc.Height);
}

void D3D::Release()
{
	DeleteBackBuffer();

	//	파이프 라인의 모든 설정을
	//	초기화 시킨다.
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
	//		   다이렉트X 그래픽스 하부구조
	//	- DirectX 11등의 그래픽스 기능이나
	//	- 어플리케이션으로부터 오는 표시를 받아
	//	- 커널모드 드라이버나 하드웨어와 주고 받는 역할
	///////////////////////////////////////////////////////////////////////////////

	HRESULT hr;
	IDXGIFactory *factory = NULL;
	IDXGIAdapter *adapter = NULL;
	IDXGIOutput *output = NULL;
	DXGI_MODE_DESC *displayModeList = NULL;
	//	1. DXGI의 기능을 사용하려면 우선 IDXGIFactory인터페이스를 얻어와야한다.
	{
		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		assert(SUCCEEDED(hr));
	}
	//	2. 그래픽 카드 정보 취득
	{
		//	그래픽 카드 관련 정보

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
		//	CPU와 공유되고 있지 않은 비디오 메모리 바이트 수
		wprintf_s(L"DedicatedVideoMemory : %dMB\n", adapterDesc.DedicatedVideoMemory / 1024 / 1024);
		//	GPU와 공유되고 있지 않은 시스템 메모리 바이트 수
		wprintf_s(L"DedicatedSystemMemory : %dMB\n", adapterDesc.DedicatedSystemMemory);
		//	공유되고 있는 시스템 메모리 바이트 수
		wprintf_s(L"SharedSystemMemory : %dMB\n\n", adapterDesc.SharedSystemMemory / 1024 / 1024);
		wprintf_s(L"-------------------------------\n\n");

		//	디스플레이 관련 정보
		hr = adapter->EnumOutputs(0, &output);
		assert(SUCCEEDED(hr));

		DXGI_OUTPUT_DESC outputDesc;
		output->GetDesc(&outputDesc);

		wprintf_s(L"<Display Info>\n\n");
		wprintf_s(L"Device Name : %s\n", outputDesc.DeviceName);
		//	데스크탑 좌표에서 출력 범위
		wprintf_s(L"DesktopCoordLeftTop : %d, %d\n", outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.top);
		wprintf_s(L"DesktopCoordRightBottom : %d, %d\n", outputDesc.DesktopCoordinates.right, outputDesc.DesktopCoordinates.bottom);
		//	출력이 데스크탑에 연관되어 있는가
		wprintf_s(L"IsAttackedToDesktop : %s\n", outputDesc.AttachedToDesktop == 0 ? L"False" : L"True");

		//	이 모드에 지원하는 GPU 전부 가져옴
		//	Display Mode의 갯수를 얻어옴
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
		//	displayModeList 초기화
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

	// ALT + ENTER 화면 모드 전환
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
	//	1. 사용할 피쳐 레벨 결정
	//	지원 되는 DX버전
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

	//	2. 디바이스와 스왑체인 생성
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	//	D3D11_CREATE_DEVICE_SINGLETHREADED	: 싱글 스레드 지원 디바이스
	//	D3D11_CREATE_DEVICE_DEBUG			: 디버그 레이어
	//	D3D11_CREATE_DEVICE_SWITCH_TO_REF	: 레퍼런스 타입으로 교체 할수 있는 디바이스
	//	D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS : 복수의 스레드가 만들어지지 않도록 한다. 일반적으로 사용하지 않음
	//	D3D11_CREATE_DEVICE_BGRA_SUPPORT	: Direct2D와 Direct3D 리소스의 상호운영을 가능하게 한다.
#if defined(_DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//	디버그 레이어를 사용 가능하게 해두면
	//	프로그램의 성능이 떨어지기 때문에
	//	릴리즈 모드에서는 사용하지 않는다.
#endif

	//	3. 스왑체인 설정
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;				//	백 버퍼의 개수
	swapChainDesc.BufferDesc.Width = 0;			//	백 버퍼의 가로, 세로
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//	8bit, 0.0 ~ 1.0
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//	화면 주사율
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

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//	백 버퍼를 어떤 용도로 사용할 것인지
	swapChainDesc.OutputWindow = d3dDesc.Handle;					//	백 버퍼를 출력할 윈도우 핸들
	swapChainDesc.SampleDesc.Count = 1;								//	멀티 샘플링 수
	swapChainDesc.SampleDesc.Quality = 0;							//	멀티 샘플링 퀄리티
	swapChainDesc.Windowed = !d3dDesc.bFullScreen;					//	윈도우모드
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//	DX를 이용한 모든 프로그램은
	//	Device를 생성하는 것이 가장 기본이다.
	//	Device와 SwapChain은 밑 함수를 통해 동시에 생성한다.
	//	이때 실제 렌더링에 사용될 백 버퍼인 DeviceContext도 같이 생성된다.
	//	DX11의 가장 핵심인 함수이다.
	HRESULT hr = D3D11CreateDeviceAndSwapChain
	(
		NULL							//	사용할 IDXGIAdapter 인터페이스
										//	그래픽 카드를 지정할 필요가 없으면 NULL로 한다.
										//	NULL로 하면 최초로 발견된 그래픽 카드가 지정된다.
		//D3D_DRIVER_TYPE_HARDWARE		:	하드웨어 드라이브 사용(DX9->HAL), Direct3D가 세부적인 부분을 제어하지 않고
		//D3D_DRIVER_TYPE_WARP			:	고속 포퍼먼스의 소프트웨어 래스터라이져
		// ==> 그래픽 카드가 피쳐레벨을 지원되지 않는 종류여도
		//		WARP 디바이스를 생성하면 피쳐레벨 9_1 ~ 10_1까지 지원한다.
		//		윈도우 비스타 이상에서만, 그리고 DX11이상에서만
		//D3D_DRIVER_TYPE_REFERENCE		:	레퍼런스 래스터라이저(DX9->REF), 테스트의 목적으로만 사용
		//	==> 모든 피쳐레벨을 지원한다.
		//		하지만 SDK가 설치되어있는 환경에서만 가능한다.
		//D3D_DRIVER_TYPE_SOFTWARE		:	예약만 되어있고, 사용하지 않음
		//D3D_DRIVER_TYPE_NULL			:	렌더링이 불가능한 레퍼런스 드라이버
		//D3D_DRIVER_TYPE_UNKNOWN		:	종류 불명
		, D3D_DRIVER_TYPE_HARDWARE		//	DX11 디바이스 종류
		, NULL							//
		, creationFlags					//	디바이스 플래그(API 레이어)
		, featureLevels					//	사용 가능한 DX버전
		, 1								//	피쳐 레벨 배열 갯수
		, D3D11_SDK_VERSION				//	SDK 버전
		, &swapChainDesc				//	스왑체인 구조체
		, &swapChain					//	넘겨 받을 스왑체인 인터페이스
		, &device						//	넘겨 받을 디바이스 인터페이스
		, NULL							//	피쳐 레벨을 얻어낼 포인터
		, &deviceContext				//	넘겨 받을 디바이스 컨텍스트(DC)
	);
	assert(SUCCEEDED(hr));	//	그래픽 카드가 위 DX버전을 지원하지 않을 때
}

void D3D::CreateBackBuffer(float width, float height)
{
	//	디바이스, DC, 스왑체인은 설정했어도
	//	스왑체인의 백버퍼가 렌더 타겟으로 설정 되어있지 않다.
	//	그래서 스왑체인으로부터 백버퍼를 얻어와서
	//	디바이스의 렌더 타겟으로 설정해야한다.
	HRESULT hr;

	//	스왑 체인의 백버퍼를 D3DX 렌더타겟에 설정
	{
		//	스왑체인으로부터 백 버퍼를 얻어옴
		//	백 버퍼는 기본적으로 2D텍스쳐로 만들어져 있기 때문에
		//	ID3D11Texture2D로 백 버퍼를 받아온다.
		ID3D11Texture2D *pBackBuffer = NULL;
		hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		assert(SUCCEEDED(hr));

		//	텍스쳐는 파이프라인으로 부터 뷰를 통해 접근이 가능함
		//	렌더 타겟도 텍스쳐의 일종이기 때문에 뷰를 이용해 접근해야한다.

		//	렌더 타겟 뷰 생성
		hr = device->CreateRenderTargetView
		(
			pBackBuffer,		//	뷰에서 엑세스 하는 포인터
			NULL,				//	렌더 타겟 뷰 설정
			&renderTargetView	//	변환된 RTV를 받아올 변수의 포인터
		);
		assert(SUCCEEDED(hr));

		//	렌더 타겟 뷰가 생성되고 나면
		//	직접 백버퍼의 포인터에 접근하지 않고
		//	이 렌더 타겟 뷰를 사용한다.
		//	그래서 해제해 준다.
		SAFE_RELEASE(pBackBuffer);
	}

	// 깊이/스텐실 버퍼 설정
	//	: DXD11은 깊이 버퍼와 스텐실 버퍼는 1개의 리소스를 공유한다.
	//	: DxD11에서는 깊이/스텐실 버퍼는 텍스쳐 리소스의 한 종류이다.
	{
		//	깊이/스텐실 텍스쳐 설정
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		//	밉맵 레벨 : Anti-Aliasing 기법 중 하나이다.
		//	텍스쳐의 LOD같은 개념이다.
		//	텍스쳐를 하나 만들면 내부적으로 여러 크기
		//	단계의 텍스쳐를 만들어 두는 것
		//	상황에 맞게 맞는 크기의 텍스쳐를 사용
		desc.MipLevels = 1;
		desc.ArraySize = 1;								//	배열 사이즈
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//	포맷 Depth(24bit), Stencil(8bit)
		desc.SampleDesc.Count = 1;						//	멀티 샘플링 수
		desc.SampleDesc.Quality = 0;					//	멀티 샘플링 퀄리티
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		//	깊이/스텐실 버퍼로 사용하겠다.
		desc.CPUAccessFlags = 0;						//	CPU로 부터는 접근하지 않겠다.
		desc.MiscFlags = 0;								//	그 외 설정 없음

														//	텍스쳐 생성
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &backBuffer);
		assert(SUCCEEDED(hr));
	}

	//	텍스쳐는 텍스쳐 리소스의 한 종류 이므로
	//	파이프라인에 적용하기 위해서는 뷰를 사용한다.
	//	깊이/스텐실 뷰(Depth_Stencil_View)를 설정
	//
	{
		//	백 버퍼와 항상 같이 있음
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		//	텍스쳐 생성에 사용한 포맷 그대로 사용
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//	2D 텍스쳐 사용
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateDepthStencilView(backBuffer, &desc, &depthStencilView);
		assert(SUCCEEDED(hr));

		//	렌더 타겟 뷰 설정
		SetRenderTarget(renderTargetView, depthStencilView);
	}
}

void D3D::DeleteBackBuffer()
{
	SAFE_RELEASE(depthStencilView);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(backBuffer);
}