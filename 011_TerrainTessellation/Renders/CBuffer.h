#pragma once

class CBuffer
{
public:
	friend class CBuffers;

public:
	CBuffer(class Shader* shader, string bufferName, void* pData, UINT dataSize);
	~CBuffer();

	void Change();

private:
	void Changed();

private:
	string name;
	class Shader* shader;
	
	bool bChanged;
	ID3D11Buffer* buffer;

	void* data;
	UINT dataSize;
};

class CBuffers
{
public:
	friend class CBuffer;

public:
	static void Update();

private:
	static vector<CBuffer *> buffers;
};