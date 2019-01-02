#pragma once

class Texture
{
public:
	friend class Textures;

public:
	static void SaveFile(wstring file, ID3D11Texture2D* src);
	static UINT ReadPixel(ID3D11Texture2D* src, const UINT & xPos, const UINT & yPos, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	static D3DXVECTOR4 ReadPixel128(ID3D11Texture2D* src, const UINT & xPos, const UINT & yPos);
	static D3D11_TEXTURE2D_DESC ReadPixels(ID3D11Texture2D* src, DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);
	static void WritePixels(ID3D11Texture2D* src, DXGI_FORMAT writeFormat, vector<D3DXCOLOR>& pixels);

public:
	Texture(wstring file, D3DX11_IMAGE_LOAD_INFO* loadInfo = NULL);
	~Texture();

	wstring GetFile() { return file; }

	UINT GetWidth() { return metaData.width; }
	UINT GetHeight() { return metaData.height; }

	D3D11_TEXTURE2D_DESC ReadPixels(DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);


	void WritePixels(DXGI_FORMAT writeFormat, vector<D3DXCOLOR>& pixels);
	void SaveFile(wstring file);

	void GetImageInfo(DirectX::TexMetadata* data)
	{
		*data = metaData;
	}

	ID3D11ShaderResourceView* SRV() { return view; }

private:
	wstring file;

	DirectX::TexMetadata metaData;
	ID3D11ShaderResourceView* view;
};

struct TextureDesc
{
	wstring file;
	UINT width, height;
	DirectX::TexMetadata metaData;
	ID3D11ShaderResourceView* view;

	bool operator==(const TextureDesc& desc)
	{
		bool b = true;
		b &= file == desc.file;
		b &= width == desc.width;
		b &= height == desc.height;

		return b;
	}
};

class Textures
{
public:
	friend class Texture;

public:
	static void Create();
	static void Delete();

private:
	static void Load(Texture* texture, D3DX11_IMAGE_LOAD_INFO* loadInfo = NULL);

private:
	static vector<TextureDesc> descs;
};

class TextureArray
{
public:
	TextureArray(vector<wstring>& names, UINT width = 256, UINT height = 256, UINT mipLevels = 1);
	~TextureArray();

	ID3D11ShaderResourceView* GetSRV() { return srv; }
	ID3D11ShaderResourceView* GetView() { return srv; }

private:
	vector<ID3D11Texture2D *> CreateTextures(vector<wstring>& names, UINT width, UINT height, UINT mipLevels);

private:
	ID3D11ShaderResourceView * srv;
};