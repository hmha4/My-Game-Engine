#include "Framework.h"
#include "CsResource.h"

// --------------------------------------------------------------------- //
//  CsResourceBase
// --------------------------------------------------------------------- //
CsResourceBase::CsResourceBase()
	: srv(NULL), uav(NULL)
{
}

CsResourceBase::~CsResourceBase()
{
	SAFE_RELEASE(srv);
	SAFE_RELEASE(uav);
}

// --------------------------------------------------------------------- //
//  CsResource
// --------------------------------------------------------------------- //
CsResource::CsResource(UINT stride, UINT count, void* initData)
	: buffer(NULL), result(NULL)
{
	byteWidth = stride * count;

	CreateBuffer(stride, count, initData, buffer);
	CreateBuffer(stride, count, NULL, result);

	CreateSRV();
	CreateUAV();
}

CsResource::~CsResource()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(result);
}

void CsResource::Read(OUT void * data)
{
	D3D::GetDC()->CopyResource(result, buffer);

	D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
	D3D::GetDC()->Map(result, 0, D3D11_MAP_READ, 0, &subResource);
	{
		memcpy(data, subResource.pData, byteWidth);
	}
	D3D::GetDC()->Unmap(result, 0);
}

void CsResource::CreateBuffer(UINT stride, UINT count, void* initData, ID3D11Buffer* buffer)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	HRESULT hr;
	if (this->buffer == NULL)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		desc.ByteWidth = byteWidth;
		desc.Usage = D3D11_USAGE_DEFAULT;

		if (initData != NULL)
		{
			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = initData;

			hr = D3D::GetDevice()->CreateBuffer(&desc, &subResource, &this->buffer);
		}
		else
			hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &this->buffer);
	}
	else
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.ByteWidth = byteWidth;
		desc.StructureByteStride = stride;

		hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &result);
	}
	assert(SUCCEEDED(hr));
}

void CsResource::CreateSRV()
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

	HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &srv);
	assert(SUCCEEDED(hr));
}

void CsResource::CreateUAV()
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

	HRESULT hr = D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav);
	assert(SUCCEEDED(hr));
}

// --------------------------------------------------------------------- //
//  CsAppend
// --------------------------------------------------------------------- //
CsAppend::CsAppend(UINT stride, UINT count, void * initData)
	: buffer(NULL), result(NULL), count(count)
{
	byteWidth = stride * count;

	CreateBuffer(stride, count, initData, buffer);
	CreateBuffer(stride, count, NULL, result);

	CreateUAV();
}

CsAppend::~CsAppend()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(result);
}

void CsAppend::Write(void * data)
{
	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = data;

	D3D::GetDC()->UpdateSubresource(buffer, 0, NULL, data, byteWidth, 0);
}

void CsAppend::Read(void * data)
{
	D3D::GetDC()->CopyResource(result, buffer);

	D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
	D3D::GetDC()->Map(result, 0, D3D11_MAP_READ, 0, &subResource);
	{
		memcpy(data, subResource.pData, byteWidth);
	}
	D3D::GetDC()->Unmap(result, 0);
}

void CsAppend::CreateBuffer(UINT stride, UINT count, void * initData, ID3D11Buffer * buffer)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	HRESULT hr;
	if (this->buffer == NULL)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		desc.ByteWidth = byteWidth;
		desc.Usage = D3D11_USAGE_DEFAULT;

		if (initData != NULL)
		{
			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = initData;

			hr = D3D::GetDevice()->CreateBuffer(&desc, &subResource, &this->buffer);
		}
		else
			hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &this->buffer);
	}
	else
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.ByteWidth = byteWidth;
		desc.StructureByteStride = stride;

		hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &result);
	}
	assert(SUCCEEDED(hr));
}

void CsAppend::CreateUAV()
{
	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;

	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = count;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

	HRESULT hr = D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav);
	assert(SUCCEEDED(hr));
}