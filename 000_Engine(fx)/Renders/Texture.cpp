#include "stdafx.h"
#include "Texture.h"

using namespace DirectX;

vector<TextureDesc> Textures::descs;

Texture::Texture(wstring file, D3DX11_IMAGE_LOAD_INFO* loadInfo)
	: file(file)
{
	Textures::Load(this, loadInfo);
}

Texture::~Texture()
{
}

D3D11_TEXTURE2D_DESC Texture::ReadPixels(DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels)
{
	ID3D11Texture2D* srcTexture;
	view->GetResource((ID3D11Resource **)&srcTexture);

	return ReadPixels(srcTexture, readFormat, pixels);
}

void Texture::WritePixels(vector<D3DXCOLOR> pixels, wstring fileName)
{
	ID3D11Texture2D* srcTexture;
	view->GetResource((ID3D11Resource **)&srcTexture);

	WritePixels(srcTexture, pixels, fileName);
}

void Texture::SaveFile(wstring file)
{
	ID3D11Texture2D* srcTexture;
	view->GetResource((ID3D11Resource **)&srcTexture);

	SaveFile(file, srcTexture);
}

void Texture::SaveFile(wstring file, ID3D11Texture2D * src)
{
	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	ID3D11Texture2D* dest;
	D3D11_TEXTURE2D_DESC destDesc;
	ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
	destDesc.Width = srcDesc.Width;
	destDesc.Height = srcDesc.Height;
	destDesc.MipLevels = 1;
	destDesc.ArraySize = 1;
	destDesc.Format = srcDesc.Format;
	destDesc.SampleDesc = srcDesc.SampleDesc;
	destDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	destDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT hr;
	hr = D3D::GetDevice()->CreateTexture2D(&destDesc, NULL, &dest);
	assert(SUCCEEDED(hr));

	hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), src, NULL, dest);
	assert(SUCCEEDED(hr));

	hr = D3DX11SaveTextureToFile(D3D::GetDC(), dest, D3DX11_IFF_PNG, file.c_str());
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(dest);
}

D3D11_TEXTURE2D_DESC Texture::ReadPixels(ID3D11Texture2D * src, DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels)
{
	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = srcDesc.Width;
	desc.Height = srcDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = readFormat;
	desc.SampleDesc = srcDesc.SampleDesc;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;


	HRESULT hr;

	ID3D11Texture2D* texture;
	hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture);
	assert(SUCCEEDED(hr));

	hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), src, NULL, texture);
	assert(SUCCEEDED(hr));


	D3D11_MAPPED_SUBRESOURCE map;
	UINT* colors = new UINT[desc.Width * desc.Height];
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_READ, NULL, &map);
	{
		memcpy(colors, map.pData, sizeof(UINT) * desc.Width * desc.Height);
	}
	D3D::GetDC()->Unmap(texture, 0);


	pixels->reserve(desc.Width * desc.Height);
	for (UINT y = 0; y < desc.Height; y++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			UINT index = desc.Width * y + x;

			CONST FLOAT f = 1.0f / 255.0f;
			float r = f * (float)((0xFF000000 & colors[index]) >> 24);
			float g = f * (float)((0x00FF0000 & colors[index]) >> 16);
			float b = f * (float)((0x0000FF00 & colors[index]) >> 8);
			float a = f * (float)((0x000000FF & colors[index]) >> 0);

			pixels->push_back(D3DXCOLOR(a, b, g, r));
		}
	}

	SAFE_DELETE_ARRAY(colors);
	SAFE_RELEASE(src);
	SAFE_RELEASE(texture);

	return desc;
}

D3D11_TEXTURE2D_DESC Texture::WritePixels(ID3D11Texture2D * src, vector<D3DXCOLOR> pixels, wstring fileName)
{
	ID3D11Texture2D* texture;
	HRESULT hr;

	D3D11_TEXTURE2D_DESC destDesc;
	ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
	destDesc.Width = 256;
	destDesc.Height = 256;
	destDesc.MipLevels = destDesc.ArraySize = 1;
	destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	destDesc.SampleDesc.Count = 1;
	destDesc.Usage = D3D11_USAGE_STAGING;
	destDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	destDesc.MiscFlags = 0;

	hr = D3D::GetDevice()->CreateTexture2D(&destDesc, NULL, &texture);
	assert(SUCCEEDED(hr));

	D3D11_MAPPED_SUBRESOURCE map;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_WRITE, NULL, &map);
	{
		vector<UINT> pixel;
		for (UINT y = 0; y < destDesc.Height; y++)
		{
			for (UINT x = 0; x < destDesc.Width; x++)
			{
				UINT index = destDesc.Width * y + x;

				UINT r = UINT(pixels[index].a * 255.0f);
				UINT g = UINT(pixels[index].b * 255.0f);
				UINT b = UINT(pixels[index].g * 255.0f);
				UINT a = UINT(pixels[index].r * 255.0f);

				UINT rgba = (r << 24) + (g << 16) + (b << 8) + (a);

				pixel.push_back(rgba);
			}
		}

		memcpy(map.pData, &pixel[0], sizeof(UINT)*destDesc.Width * destDesc.Height);
	}
	D3D::GetDC()->Unmap(texture, 0);

	hr = D3DX11SaveTextureToFile(D3D::GetDC(), texture, D3DX11_IFF_PNG, fileName.c_str());
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(src);
	SAFE_RELEASE(texture);

	return destDesc;
}

void Textures::Create()
{

}

void Textures::Delete()
{
	for (TextureDesc desc : descs)
		SAFE_RELEASE(desc.view);
}

void Textures::Load(Texture * texture, D3DX11_IMAGE_LOAD_INFO * loadInfo)
{
	HRESULT hr;

	TexMetadata metaData;
	wstring ext = Path::GetExtension(texture->file);
	if (ext == L"tga")
	{
		hr = GetMetadataFromTGAFile(texture->file.c_str(), metaData);
		assert(SUCCEEDED(hr));
	}
	else if (ext == L"dds")
	{
		hr = GetMetadataFromDDSFile(texture->file.c_str(), DDS_FLAGS_NONE, metaData);
		assert(SUCCEEDED(hr));
	}
	else if (ext == L"hdr")
	{
		assert(false);
	}
	else
	{
		hr = GetMetadataFromWICFile(texture->file.c_str(), WIC_FLAGS_NONE, metaData);
		assert(SUCCEEDED(hr));
	}

	UINT width = metaData.width;
	UINT height = metaData.height;

	if (loadInfo != NULL)
	{
		width = loadInfo->Width;
		height = loadInfo->Height;

		metaData.width = loadInfo->Width;
		metaData.height = loadInfo->Height;
	}


	TextureDesc desc;
	desc.file = texture->file;
	desc.width = width;
	desc.height = height;

	TextureDesc exist;
	bool bExist = false;
	for (TextureDesc temp : descs)
	{
		if (desc == temp)
		{
			bExist = true;
			exist = temp;

			break;
		}
	}

	if (bExist == true)
	{
		texture->metaData = exist.metaData;
		texture->view = exist.view;
	}
	else
	{
		ScratchImage image;
		if (ext == L"tga")
		{
			hr = LoadFromTGAFile(texture->file.c_str(), &metaData, image);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"dds")
		{
			hr = LoadFromDDSFile(texture->file.c_str(), DDS_FLAGS_NONE, &metaData, image);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"hdr")
		{
			assert(false);
		}
		else
		{
			hr = LoadFromWICFile(texture->file.c_str(), WIC_FLAGS_NONE, &metaData, image);
			assert(SUCCEEDED(hr));
		}

		ID3D11ShaderResourceView* view;

		hr = DirectX::CreateShaderResourceView(D3D::GetDevice(), image.GetImages(), image.GetImageCount(), metaData, &view);
		assert(SUCCEEDED(hr));

		desc.file = texture->file;
		desc.width = metaData.width;
		desc.height = metaData.height;
		desc.view = view;

		texture->view = view;
		texture->metaData = metaData;

		descs.push_back(desc);
	}
}