#include "stdafx.h"
#include "Blur.h"

Blur::Blur(wstring shaderFile, UINT width, UINT height, DXGI_FORMAT format, int blurCount)
	: width(width), height(height)
	, blurCount(blurCount)
{
	shader = new Shader(shaderFile, true);
	shader->AsScalar("BlurRadius")->SetInt(blurCount);

	HRESULT hr;
	ID3D11Texture2D * texture;

	//	Create Texture2D
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture);
		assert(SUCCEEDED(hr));
	}

	//	Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		desc.Format = format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;

		hr = D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &srv);
		assert(SUCCEEDED(hr));
	}

	//	Create UAV
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

		desc.Format = format;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateUnorderedAccessView(texture, &desc, &uav);
		assert(SUCCEEDED(hr));
	}

	SAFE_RELEASE(texture);
}

Blur::~Blur()
{
	SAFE_DELETE(shader);

	SAFE_RELEASE(srv);
	SAFE_RELEASE(uav);
}

void Blur::SetWeights(float sigma)
{
	float d = 2.0f * powf(sigma, 2);
	
	float*weights = new float[blurCount * 2 + 1];
	float sum = 0.0f;

	for (int i = -blurCount; i <= blurCount; i++)
	{
		weights[i + blurCount] = expf((float)-i * (float)i / d);

		sum += weights[i + blurCount];
	}

	for (int i = 0; i < blurCount * 2 + 1; i++)
		weights[i] /= sum;

	shader->AsScalar("Weights")->SetFloatArray(weights, 0, blurCount * 2 + 1);
	SAFE_DELETE_ARRAY(weights);
}

void Blur::Dispatch(ID3D11ShaderResourceView * inputSRV, ID3D11UnorderedAccessView * outputUAV)
{
	//	ceil : 올림
	//	floor : 내림
	//	round : 반올림
	UINT groupWidth = (UINT)ceilf((float)width / 256.0f);
	UINT groupHeight = (UINT)ceilf((float)height / 256.0f);

	for (int i = 0; i < blurCount; i++)
	{
		shader->AsShaderResource("Input")->SetResource(inputSRV);
		shader->AsUAV("Output")->SetUnorderedAccessView(uav);
		shader->Dispatch(0, 0, groupWidth, height, 1);

		shader->AsShaderResource("Input")->SetResource(srv);
		shader->AsUAV("Output")->SetUnorderedAccessView(outputUAV);
		shader->Dispatch(0, 1, width, groupHeight, 1);
	}
}
