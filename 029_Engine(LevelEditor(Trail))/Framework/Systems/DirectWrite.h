#pragma once

struct FontBrushDesc;
struct FontTextDesc;

class DirectWrite
{
public:
	static void Create();
	static void Delete();

	static DirectWrite * Get();
	static ID2D1DeviceContext * GetDC() { return deviceContext; }

	static void CreateSurface();
	static void DeleteSurface();

private:
	static DirectWrite * instance;

private:
	DirectWrite();
	~DirectWrite();

private:
	ID2D1Factory1 * factory;
	static IDWriteFactory * writeFactory;

	ID2D1Device * device;
	static ID2D1DeviceContext * deviceContext;

	static ID2D1Bitmap1 * targetBitmap;
	static IDXGISurface * dxgiSurface;

private:
	static vector<FontBrushDesc> fontBrush;
	static vector<FontTextDesc> fontText;

public:
	static void RenderText
	(
		wstring text,
		RECT rect,
		float size = 20.0f,
		wstring font = L"µ¸¿òÃ¼",
		D3DXCOLOR color = D3DXCOLOR(1, 1, 1, 1),
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL
	);

	RECT Get2DPosition(D3DXMATRIX w, D3DXMATRIX v, D3DXMATRIX p);
};

struct FontBrushDesc
{
	D3DXCOLOR Color;
	ID2D1SolidColorBrush * Brush = NULL;

	bool operator==(const FontBrushDesc& vec)
	{
		return Color.r == vec.Color.r &&
			Color.g == vec.Color.g &&
			Color.b == vec.Color.b &&
			Color.a == vec.Color.a;
	}

	void Release()
	{
		SAFE_RELEASE(Brush);
	}
};

struct FontTextDesc
{
	wstring Font;
	float FontSize;

	DWRITE_FONT_WEIGHT Weight;
	DWRITE_FONT_STYLE Style;
	DWRITE_FONT_STRETCH Stretch;

	IDWriteTextFormat * Format = NULL;

	bool operator==(const FontTextDesc& val)
	{
		bool b = true;
		b &= Font == val.Font;
		b &= FontSize == val.FontSize;
		b &= Weight == val.Weight;
		b &= Style == val.Style;
		b &= Stretch == val.Stretch;

		return b;
	}

	void Release()
	{
		SAFE_RELEASE(Format);
	}
};