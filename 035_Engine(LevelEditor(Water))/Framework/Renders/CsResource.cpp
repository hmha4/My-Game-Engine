#include "Framework.h"
#include "CsResource.h"

void CsResource::CreateRawBuffer(UINT size, void * initData, ID3D11Buffer ** buffer, bool cpuWrite, bool gpuWrite)
{
	// --------------------------------------------------------------------- //
	//	D3D11_BUFFER_DESC
	//	- ByteWidth : size of the buffer in bytes
	//	- Usage : Identify how the buffer is expected to be read from and written to
	//	- BindFlags : Identify how the buffer will be bound to the pipeline
	//	- MiscFlags : Miscellaneous flags or 0 if unused
	//	- CPUAccessFlags : CPU access flag or 0 if no cpu access is necessary
	// --------------------------------------------------------------------- //
	D3D11_BUFFER_DESC desc = { 0 };
	desc.ByteWidth = size;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	//	IMMUTABLE : GPU(read only) & CPU(X)
	if (cpuWrite == false && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	//	DYNAMIC : GPU(read only) & CPU(write only)
	else if (cpuWrite == true && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	//	DEFAULT : GPU(read + write) & CPU(X)
	else if (cpuWrite == false && gpuWrite == true)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
	}

	HRESULT hr = NULL;

	if (initData != NULL)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initData;

		hr = D3D::GetDevice()->CreateBuffer(&desc, &data, buffer);
	}
	else
	{
		hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, buffer);
	}
	assert(SUCCEEDED(hr));
}

void CsResource::CreateStructuredBuffer(UINT size, UINT count, void * initData, ID3D11Buffer ** buffer, bool cpuWrite, bool gpuWrite)
{
	D3D11_BUFFER_DESC desc = { 0 };
	desc.ByteWidth = size * count;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = size;

	//	IMMUTABLE : GPU(read only) & CPU(X)
	if (cpuWrite == false && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	//	DYNAMIC : GPU(read only) & CPU(write only)
	else if (cpuWrite == true && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	//	DEFAULT : GPU(read + write) & CPU(X)
	else if (cpuWrite == false && gpuWrite == true)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
	}

	HRESULT hr;
	if (initData != NULL)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initData;

		hr = D3D::GetDevice()->CreateBuffer(&desc, &data, buffer);
	}
	else
	{
		hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, buffer);
	}
	assert(SUCCEEDED(hr));
}

void CsResource::CreateTexture(UINT width, UINT height, void * initData, ID3D11Texture2D ** texture, bool cpuWrite, bool gpuWrite, DXGI_FORMAT format, UINT pitch)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	//	밉맵 레벨 : Anti-Aliasing 기법 중 하나이다.
	//	텍스쳐의 LOD같은 개념이다.
	//	텍스쳐를 하나 만들면 내부적으로 여러 크기
	//	단계의 텍스쳐를 만들어 두는 것
	//	상황에 맞게 맞는 크기의 텍스쳐를 사용
	desc.MipLevels = 1;
	desc.ArraySize = 1;								//	배열 사이즈(큐브맵 만들때 사용)
	desc.Format = format;							//	텍스쳐 포맷
	desc.SampleDesc.Count = 1;						//	픽셀 당 멀티샘플 할 수
	desc.SampleDesc.Quality = 0;					//	이미지 퀄리티(높을 수록 성능 저하)
	desc.MiscFlags = 0;

	//	IMMUTABLE : GPU(read only) & CPU(X)
	if (cpuWrite == false && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	//	DYNAMIC : GPU(read only) & CPU(write only)
	else if (cpuWrite == true && gpuWrite == false)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	//	DEFAULT : GPU(read + write) & CPU(X)
	else if (cpuWrite == false && gpuWrite == true)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
	}

	HRESULT hr;

	if (initData != NULL)
	{
		D3D11_SUBRESOURCE_DATA data{ 0 };
		data.pSysMem = initData;
		data.SysMemPitch = pitch;

		hr = D3D::GetDevice()->CreateTexture2D(&desc, &data, texture);
	}
	else
	{
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, texture);
	}

	assert(SUCCEEDED(hr));
}

ID3D11Texture2D*CsResource::WriteTexture(DXGI_FORMAT writeFormat, ID3D11Texture2D*copyTex)
{
	D3D11_TEXTURE2D_DESC writeTexturedesc;
	D3D11_TEXTURE2D_DESC copyTexturedesc;
	copyTex->GetDesc(&copyTexturedesc);
	ZeroMemory(&writeTexturedesc, sizeof(D3D11_TEXTURE2D_DESC));

	writeTexturedesc.Width = copyTexturedesc.Width;
	writeTexturedesc.Height = copyTexturedesc.Height;
	writeTexturedesc.MipLevels = 1;
	writeTexturedesc.ArraySize = 1;
	writeTexturedesc.Format = writeFormat;
	writeTexturedesc.SampleDesc = copyTexturedesc.SampleDesc;
	writeTexturedesc.SampleDesc.Count = 1;
	writeTexturedesc.Usage = D3D11_USAGE_STAGING;
	writeTexturedesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//writeTexturedesc.MiscFlags = copyTexturedesc.MiscFlags;

	HRESULT hr;

	ID3D11Texture2D* texture;
	hr = D3D::GetDevice()->CreateTexture2D(&writeTexturedesc, NULL, &texture);
	assert(SUCCEEDED(hr));

	hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), copyTex, NULL, texture);
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(copyTex);

	return texture;
}

void CsResource::CreateSRV(ID3D11Buffer * buffer, ID3D11ShaderResourceView ** srv)
{
	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;	//	BufferEx 여야만 structured buffer에 들어감
	srvDesc.BufferEx.FirstElement = 0;

	//	 & : == 이랑 같음
	if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS/*언어도더드 뷰랑 같음*/)
	{
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.BufferEx.NumElements = desc.ByteWidth / 4;
	}
	else if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
	}

	HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, srv);
	assert(SUCCEEDED(hr));
}

void CsResource::CreateSRV(ID3D11Texture2D * texture, ID3D11ShaderResourceView ** srv, DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
		srvDesc.Format = desc.Format;
	else
		srvDesc.Format = format;

	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, srv);
	assert(SUCCEEDED(hr));
}

void CsResource::CreateUAV(ID3D11Buffer * buffer, ID3D11UnorderedAccessView ** uav)
{
	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;

	//	 & : == 이랑 같음
	if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS/*언어도더드 뷰랑 같음*/)
	{
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
	}
	else if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
	}

	HRESULT hr = D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, uav);
	assert(SUCCEEDED(hr));
}

void CsResource::CreateUAV(ID3D11Texture2D * texture, ID3D11UnorderedAccessView ** uav)
{
	D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = desc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	HRESULT hr = D3D::GetDevice()->CreateUnorderedAccessView(texture, &uavDesc, uav);
	assert(SUCCEEDED(hr));
}

ID3D11Buffer * CsResource::CreateAndCopyBuffer(ID3D11Buffer * src)
{
	ID3D11Buffer * dest;

	D3D11_BUFFER_DESC desc = { 0 };
	src->GetDesc(&desc);

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &dest);
	assert(SUCCEEDED(hr));

	D3D::GetDC()->CopyResource(dest, src);

	return dest;
}