#include "stdafx.h"
#include "DirectWrite.h"

DirectWrite * DirectWrite::instance = NULL;
ID2D1DeviceContext * DirectWrite::deviceContext = NULL;
IDWriteFactory * DirectWrite::writeFactory = NULL;
IDXGISurface * DirectWrite::dxgiSurface = NULL;
ID2D1Bitmap1 * DirectWrite::targetBitmap = NULL;

vector<FontBrushDesc> DirectWrite::fontBrush;
vector<FontTextDesc> DirectWrite::fontText;

void DirectWrite::Create()
{
	assert(instance == NULL);

	instance = new DirectWrite();
}

void DirectWrite::Delete()
{
	assert(instance != NULL);

	SAFE_DELETE(instance);
}

DirectWrite * DirectWrite::Get()
{
	return instance;
}

void DirectWrite::CreateSurface()
{
	HRESULT hr = D3D::GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), (void **)&dxgiSurface);
	assert(SUCCEEDED(hr));

	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96;
	bp.dpiY = 96;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = NULL;

	deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bp, &targetBitmap);
	assert(SUCCEEDED(hr));

	deviceContext->SetTarget(targetBitmap);
}

void DirectWrite::DeleteSurface()
{
	deviceContext->SetTarget(NULL);

	for (FontBrushDesc desc : fontBrush)
		desc.Release();
	fontBrush.clear();

	for (FontTextDesc desc : fontText)
		desc.Release();
	fontText.clear();

	SAFE_RELEASE(targetBitmap);
	SAFE_RELEASE(dxgiSurface);
}

DirectWrite::DirectWrite()
{
	HRESULT hr;
	hr = DWriteCreateFactory
	(
		DWRITE_FACTORY_TYPE_SHARED, 
		__uuidof(IDWriteFactory),		//	클래스 자료형의 ID
		(IUnknown **)&writeFactory
	);
	assert(SUCCEEDED(hr));

	D2D1_FACTORY_OPTIONS option;
	option.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
	assert(SUCCEEDED(hr));

	//	DXGI : GPU가 어떻 것이던 동일한 동작을 하게끔 해줌
	IDXGIDevice * dxgiDevice;
	hr = D3D::GetDevice()->QueryInterface(&dxgiDevice);
	assert(SUCCEEDED(hr));

	hr = factory->CreateDevice(dxgiDevice, &device);
	assert(SUCCEEDED(hr));

	hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &deviceContext);
	assert(SUCCEEDED(hr));

	CreateSurface();
}

DirectWrite::~DirectWrite()
{
	DeleteSurface();
	
	SAFE_RELEASE(factory);
	SAFE_RELEASE(writeFactory);

	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(device);
}

RECT DirectWrite::Get2DPosition(D3DXMATRIX w, D3DXMATRIX v, D3DXMATRIX p)
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	D3DXMATRIX WVP;
	WVP = w * v * p;

	D3DXVECTOR4 worldToScreen(0, 0, 0, 1);
	D3DXVec4Transform(&worldToScreen, &worldToScreen, &WVP);

	//	NDC 공간으로 변환
	float wScreenX = worldToScreen.x / worldToScreen.w;
	float wScreenY = worldToScreen.y / worldToScreen.w;
	float wScreenZ = worldToScreen.z / worldToScreen.w;

	//	-1~1 구간을 0~1구간으로 변환
	float nScreenX = (wScreenX + 1) * 0.5f;
	float nScreenY = 1.0f - (wScreenY + 1) * 0.5f;

	//	최종 화면의 좌표
	float resultX = nScreenX * desc.Width;
	float resultY = nScreenY * desc.Height;

	RECT rect;
	rect.left = (LONG)resultX;
	rect.top = (LONG)resultY - 20;
	rect.right = (LONG)resultX + 1000;
	rect.bottom = (LONG)resultY;
	
	
	return rect;
}

void DirectWrite::RenderText(wstring text, RECT rect, float size, wstring font, D3DXCOLOR color, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch)
{
	FontBrushDesc brushDesc;
	brushDesc.Color = color;

	FontBrushDesc * findBrush = NULL;
	for (FontBrushDesc& desc : fontBrush)
	{
		if (desc == brushDesc)
		{
			findBrush = &desc;
			break;
		}
	}

	if (findBrush == NULL)
	{
		D2D1::ColorF colorF = D2D1::ColorF(color.r, color.g, color.b);

		deviceContext->CreateSolidColorBrush(colorF, &brushDesc.Brush);

		fontBrush.push_back(brushDesc);
		findBrush = &brushDesc;
	}

	FontTextDesc textDesc;
	textDesc.Font = font;
	textDesc.FontSize = size;
	textDesc.Stretch = stretch;
	textDesc.Style = style;
	textDesc.Weight = weight;

	FontTextDesc * findText = NULL;
	for (FontTextDesc& desc : fontText)
	{
		if (desc == textDesc)
		{
			findText = &desc;
			break;
		}
	}

	if (findText == NULL)
	{
		writeFactory->CreateTextFormat
		(
			textDesc.Font.c_str(), NULL, 
			textDesc.Weight, textDesc.Style, 
			textDesc.Stretch, textDesc.FontSize,
			L"ko", &textDesc.Format
		);

		fontText.push_back(textDesc);
		findText = &textDesc;
	}

	D2D1_RECT_F rectF;
	rectF.left = (float)rect.left;
	rectF.right = (float)rect.right;
	rectF.top = (float)rect.top;
	rectF.bottom = (float)rect.bottom;


	deviceContext->DrawTextW
	(
		text.c_str(), text.size(), findText->Format, rectF, findBrush->Brush
	);
}
