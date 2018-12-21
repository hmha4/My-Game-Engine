#include "Framework.h"
#include "Texture.h"

using namespace DirectX;

// ------------------------------------------------------------------------- //
//	Texture
// ------------------------------------------------------------------------- //
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

void Texture::SaveFile(wstring file)
{
	ID3D11Texture2D* srcTexture;
	view->GetResource((ID3D11Resource **)&srcTexture);

	SaveFile(file, srcTexture);
}

// --------------------------------------------------------------------- //
//	D3D11_TEXTURE2D_DESC
//	- Width : the width of the image
//	- Height : the height of the image
//	- MipLevels : The maximum number of mipmap levels in the texture
//	- ArraySize : Number of textures in the texture array
//	- Format : Texture format(DXGI_FORMAT)
//	- SampleDesc : Structure that specifies multisampling parameters for the texture
//		- Count : The number of multisamples per pixel.
//		- Quality : The image quality level.
//	- CPUAccessFlags : Flags to specify the types of CPU access allowed
//	- Usage : Value that identifies how the texture is to be read from and written to
//	- BindFlags : Flags for binding to pipeline stages
// --------------------------------------------------------------------- //
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

// ------------------------------------------------------------------------- //
//	Textures
// ------------------------------------------------------------------------- //
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

// ------------------------------------------------------------------------- //
//	Texture Array
// ------------------------------------------------------------------------- //
TextureArray::TextureArray(vector<wstring> & names, UINT width, UINT height, UINT mipLevels)
{
	vector<ID3D11Texture2D *> textures;
	textures = CreateTextures(names, width, height, mipLevels);


	D3D11_TEXTURE2D_DESC textureDesc;
	textures[0]->GetDesc(&textureDesc);

	ID3D11Texture2D* textureArray;
	//Texture2DArray 持失
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = textureDesc.Width;
		desc.Height = textureDesc.Height;
		desc.MipLevels = textureDesc.MipLevels;
		desc.ArraySize = names.size();
		//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Format = textureDesc.Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &textureArray);
		assert(SUCCEEDED(hr));
	}


	for (UINT i = 0; i < textures.size(); i++)
	{
		for (UINT level = 0; level < textureDesc.MipLevels; level++)
		{
			D3D11_MAPPED_SUBRESOURCE subResource;
			D3D::GetDC()->Map(textures[i], level, D3D11_MAP_READ, 0, &subResource);
			{
				D3D::GetDC()->UpdateSubresource(textureArray, D3D11CalcSubresource(level, i, textureDesc.MipLevels), NULL, subResource.pData, subResource.RowPitch, subResource.DepthPitch);
			}
			D3D::GetDC()->Unmap(textures[i], level);
		}
	}

	//   Create File textures[0] -> test.png (for Test)
	//D3DX11SaveTextureToFile(D3D::GetDC(), textureArray, D3DX11_IFF_PNG, L"test.png");

	//SRV 持失
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = textureDesc.Format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MostDetailedMip = 0;
		desc.Texture2DArray.MipLevels = textureDesc.MipLevels;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = names.size();

		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(textureArray, &desc, &srv);
		assert(SUCCEEDED(hr));
	}

	for (ID3D11Texture2D* texture : textures)
		SAFE_RELEASE(texture);

	SAFE_RELEASE(textureArray);
}

TextureArray::~TextureArray()
{
	SAFE_RELEASE(srv);
}

vector<ID3D11Texture2D*> TextureArray::CreateTextures(vector<wstring>& names, UINT width, UINT height, UINT mipLevels)
{
	vector<ID3D11Texture2D *> returnTextures;
	returnTextures.resize(names.size());

	for (UINT index = 0; index < returnTextures.size(); index++)
	{
		HRESULT hr;

		TexMetadata metaData;
		wstring ext = Path::GetExtension(names[index]);
		if (ext == L"tga")
		{
			hr = GetMetadataFromTGAFile(names[index].c_str(), metaData);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"dds")
		{
			hr = GetMetadataFromDDSFile(names[index].c_str(), DDS_FLAGS_NONE, metaData);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"hdr")
		{
			assert(false);
		}
		else
		{
			hr = GetMetadataFromWICFile(names[index].c_str(), WIC_FLAGS_NONE, metaData);
			assert(SUCCEEDED(hr));
		}

		ScratchImage image;

		if (ext == L"tga")
		{
			hr = LoadFromTGAFile(names[index].c_str(), &metaData, image);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"dds")
		{
			hr = LoadFromDDSFile(names[index].c_str(), DDS_FLAGS_NONE, &metaData, image);
			assert(SUCCEEDED(hr));
		}
		else if (ext == L"hdr")
		{
			assert(false);
		}
		else
		{
			hr = LoadFromWICFile(names[index].c_str(), WIC_FLAGS_NONE, &metaData, image);
			assert(SUCCEEDED(hr));
		}

		ScratchImage resizedImage;
		hr = DirectX::Resize
		(
			image.GetImages(), image.GetImageCount(), image.GetMetadata(), width, height, TEX_FILTER_DEFAULT, resizedImage
		);
		assert(SUCCEEDED(hr));

		if (mipLevels > 1)
		{
			ScratchImage mipmapedImage;
			hr = DirectX::GenerateMipMaps
			(
				resizedImage.GetImages(), resizedImage.GetImageCount(), resizedImage.GetMetadata(), D3DX11_FILTER_NONE, mipLevels, mipmapedImage
			);
			assert(SUCCEEDED(hr));

			hr = DirectX::CreateTextureEx
			(
				D3D::GetDevice()
				, mipmapedImage.GetImages()
				, mipmapedImage.GetImageCount()
				, mipmapedImage.GetMetadata()
				, D3D11_USAGE_STAGING
				, 0
				, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE
				, 0
				, false
				, (ID3D11Resource **)&returnTextures[index]
			);
			assert(SUCCEEDED(hr));

			mipmapedImage.Release();
		}
		else
		{
			hr = DirectX::CreateTextureEx
			(
				D3D::GetDevice()
				, resizedImage.GetImages()
				, resizedImage.GetImageCount()
				, resizedImage.GetMetadata()
				, D3D11_USAGE_STAGING
				, 0
				, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE
				, 0
				, false
				, (ID3D11Resource **)&returnTextures[index]
			);
			assert(SUCCEEDED(hr));
		}

		image.Release();
		resizedImage.Release();

	}
	return returnTextures;
}