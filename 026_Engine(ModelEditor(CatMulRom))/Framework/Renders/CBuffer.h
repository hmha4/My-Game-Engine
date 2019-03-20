#pragma once

class CBuffer
{
public:
	friend class CBuffers;

public:
	CBuffer(class Effect* effect, string bufferName, void* pData, UINT dataSize);
	~CBuffer();

	void Change();

private:
	void Changed();

private:
	string name;
	class Effect* effect;

	bool bChanged;
	ID3D11Buffer* buffer;

	void* data;
	UINT dataSize;

private:
	ID3DX11EffectConstantBuffer * constantVariable;
};

class CBuffers
{
public:
	friend class CBuffer;

public:
	static void Update();
	static void Erase(CBuffer * buffer);
	static void Erase(Effect * effect);

private:
	static vector<CBuffer *> buffers;
};