#include "Framework.h"
#include "CBuffer.h"

CBuffer::CBuffer(Effect * effect, string bufferName, void * pData, UINT dataSize)
	: effect(effect), name(bufferName), data(pData), dataSize(dataSize)
{
	constantVariable = this->effect->AsConstantBuffer(name);

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = dataSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer);
	assert(SUCCEEDED(hr));

	Change();

	CBuffers::buffers.push_back(this);
}

CBuffer::~CBuffer()
{
	SAFE_RELEASE(buffer);
}

void CBuffer::Change()
{
	bChanged = true;
}

void CBuffer::Changed()
{
	if (bChanged == true)
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		HRESULT hr = D3D::GetDC()->Map
		(
			buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource
		);
		assert(SUCCEEDED(hr));

		memcpy(subResource.pData, data, dataSize);
		D3D::GetDC()->Unmap(buffer, 0);

		hr = constantVariable->SetConstantBuffer(buffer);
		assert(SUCCEEDED(hr));

		bChanged = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

vector<CBuffer *> CBuffers::buffers;

void CBuffers::Update()
{
	for (CBuffer* buffer : buffers)
	{
		buffer->Changed();
	}
}

void CBuffers::Erase(CBuffer * buffer)
{
	vector<CBuffer *>::iterator iter = buffers.begin();

	for (; iter != buffers.end();)
	{
		if ((*iter) == buffer)
		{
			SAFE_DELETE((*iter));
			iter = buffers.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void CBuffers::Erase(Effect * effect)
{
	vector<CBuffer *>::iterator iter = buffers.begin();

	for (; iter != buffers.end();)
	{
		if ((*iter)->effect == effect)
		{
			SAFE_DELETE((*iter));
			iter = buffers.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}