#pragma once

class FXAA
{
public:
	FXAA(wstring effectFile);
	~FXAA();

	void Render(bool use);
	void SetSRV(ID3D11ShaderResourceView* srv);
private:
	Effect * effect;
};