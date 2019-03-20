#pragma once

class CsResource
{
public:
	static void CreateRawBuffer(UINT size, void * initData, ID3D11Buffer ** buffer, bool cpuWrite = true, bool gpuWrite = true);
	static void CreateStructuredBuffer(UINT size, UINT count, void * initData, ID3D11Buffer ** buffer, bool cpuWrite = true, bool gpuWrite = true);
	static void CreateTexture(UINT width, UINT height, void * initData, ID3D11Texture2D ** texture, bool cpuWrite = true, bool gpuWrite = true, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	static void CreateSRV(ID3D11Buffer * buffer, ID3D11ShaderResourceView ** srv);
	static void CreateSRV(ID3D11Texture2D * texture, ID3D11ShaderResourceView ** srv);
	static void CreateUAV(ID3D11Buffer * buffer, ID3D11UnorderedAccessView ** uav);
	static void CreateUAV(ID3D11Texture2D * texture, ID3D11UnorderedAccessView ** uav);
	static ID3D11Buffer * CreateAndCopyBuffer(ID3D11Buffer * src);
};