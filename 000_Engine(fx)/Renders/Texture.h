#pragma once

class Texture
{
public:
	friend class Textures;

public:
	static void SaveFile(wstring file, ID3D11Texture2D* src);
	static D3D11_TEXTURE2D_DESC ReadPixels(ID3D11Texture2D* src, DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);
	static D3D11_TEXTURE2D_DESC WritePixels(ID3D11Texture2D* src, vector<D3DXCOLOR> pixels, wstring fileName);

public:
	Texture(wstring file, D3DX11_IMAGE_LOAD_INFO* loadInfo = NULL);
	~Texture();


	wstring GetFile() { return file; }

	UINT GetWidth() { return metaData.width; }
	UINT GetHeight() { return metaData.height; }

	D3D11_TEXTURE2D_DESC ReadPixels(DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);
	void WritePixels(vector<D3DXCOLOR> pixels, wstring fileName);

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