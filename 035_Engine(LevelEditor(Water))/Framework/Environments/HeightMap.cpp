#include "Framework.h"
#include "HeightMap.h"
#include "Utilities\BinaryFile.h"

HeightMap::HeightMap(UINT width, UINT height, float maxHeight)
	: width(width), height(height), maxHeight(maxHeight)
{
	datas = new float[width * height];
}

HeightMap::~HeightMap()
{
	SAFE_DELETE_ARRAY(datas);
}

void HeightMap::Load(wstring file)
{
	BinaryReader * r = new BinaryReader();
	BYTE * temp = new BYTE[width * height];
	r->Open(file);
	{
		r->Byte((void**)&temp, width * height);
	}
	r->Close();
	SAFE_DELETE(r);

	for (UINT i = 0; i < (width * height); i++)
		datas[i] = ((float)temp[i] / 255.0f) * maxHeight;

	Smooth();

	SAFE_DELETE_ARRAY(temp);
}

void HeightMap::Save(wstring file)
{
	BYTE * temp = new BYTE[width * height];

	for (UINT i = 0; i < (width * height); i++)
		temp[i] = (BYTE)(datas[i] / maxHeight * 255.0f);

	BinaryWriter * w = new BinaryWriter();

	w->Open(file);
	{
		w->Byte(temp, width * height);
	}
	w->Close();
	SAFE_DELETE(w);

	SAFE_DELETE_ARRAY(temp);
}

void HeightMap::Data(UINT row, UINT col, float data)
{
	if (InBounds(row, col) == false)
		return;

	datas[row * height + col] = data;
}

float HeightMap::Data(UINT row, UINT col)
{
	if (InBounds(row, col) == false)
		return 0.0f;

	return datas[row * height + col];
}

bool HeightMap::InBounds(int row, int col)
{
	return row >= 0 && row < (int)height && col >= 0 && col < (int)width;
}

void HeightMap::Smooth()
{
	float * dest = new float[width * height];
	for (UINT row = 0; row < height; row++)
	{
		for (UINT col = 0; col < width; col++)
		{
			dest[row * height + col] = Average(row, col);
		}
	}

	memcpy(datas, dest, sizeof(float) * width * height);
	SAFE_DELETE_ARRAY(dest);
}

float HeightMap::Average(UINT row, UINT col)
{
	float average = 0.0f;
	float sum = 0.0f;

	for (UINT m = row - 1; m <= row + 1; m++)
	{
		for (UINT n = col - 1; n <= col + 1; n++)
		{
			if (InBounds(m, n) == false) continue;

			average += datas[m * height + n];
			sum++;
		}
	}

	return average / sum;
}

ID3D11ShaderResourceView * HeightMap::BuildSRV()
{
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = DXGI_FORMAT_R16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	D3DXFLOAT16 * temp = new D3DXFLOAT16[width * height];
	D3DXFloat32To16Array(temp, datas, width * height);

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = temp;
	data.SysMemPitch = width * sizeof(D3DXFLOAT16);

	ID3D11Texture2D * map;
	HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, &data, &map);
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//	구조체 안에 포인터, 또는 union 가 있으면 ZeroMemory사용
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	ID3D11ShaderResourceView* srv;

	hr = D3D::GetDevice()->CreateShaderResourceView(map, &srvDesc, &srv);
	assert(SUCCEEDED(hr));

	SAFE_DELETE_ARRAY(temp);
	SAFE_RELEASE(map);

	return srv;
}