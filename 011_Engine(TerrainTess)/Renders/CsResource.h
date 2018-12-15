#pragma once

class CsResourceBase
{
public:
	CsResourceBase();
	virtual ~CsResourceBase();

	ID3D11ShaderResourceView*	SRV() { return srv; }
	ID3D11UnorderedAccessView*	UAV() { return uav; }

protected:
	ID3D11ShaderResourceView * srv;
	ID3D11UnorderedAccessView*	uav;
};

class CsResource : public CsResourceBase
{
public:
	CsResource(UINT stride, UINT count, void* initData);
	~CsResource();

	void Read(OUT void* data);

private:
	void CreateBuffer(UINT stride, UINT count, void* initData, ID3D11Buffer* buffer);
	void CreateSRV();
	void CreateUAV();

private:
	UINT byteWidth;

	ID3D11Buffer* buffer;
	ID3D11Buffer* result;
};

class CsAppend : public CsResourceBase
{
public:
	CsAppend(UINT stride, UINT count, void* initData);
	~CsAppend();

	void Write(void* data);
	void Read(void* data);

private:
	void CreateBuffer(UINT stride, UINT count, void* initData, ID3D11Buffer* buffer);
	void CreateUAV();

private:
	UINT byteWidth;
	UINT count;

	ID3D11Buffer* buffer;
	ID3D11Buffer* result;

};