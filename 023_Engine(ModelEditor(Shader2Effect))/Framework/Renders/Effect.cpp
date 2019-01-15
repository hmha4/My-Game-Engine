#include "Framework.h"
#include "Effect.h"

Effect::Effect(wstring file, bool bUseCS)
	: file(file)
{
	initialStateBlock = new StateBlock();
	{
		D3D::GetDC()->RSGetState(&initialStateBlock->RSRasterizerState);
		D3D::GetDC()->OMGetBlendState(&initialStateBlock->OMBlendState, initialStateBlock->OMBlendFactor, &initialStateBlock->OMSampleMask);
		D3D::GetDC()->OMGetDepthStencilState(&initialStateBlock->OMDepthStencilState, &initialStateBlock->OMStencilRef);
	}

	CreateEffect();

	//	Compute Shader는 Rendering Pipeline과는 별개로 작동하기 때문에
	//	공통으로 사용되는 000_Header.fx가 필요없다
	if (bUseCS == false)
	{
		Context::Get()->AddEffect(this);

		D3DXMATRIX W; D3DXMatrixIdentity(&W);
		AsMatrix("World")->SetMatrix(W);
	}
}

Effect::~Effect()
{
	Context::Get()->Erase(this);

	for (Technique& temp : techniques)
	{
		for (Pass& pass : temp.Passes)
		{
			SAFE_RELEASE(pass.InputLayout);
		}
	}

	SAFE_DELETE(initialStateBlock);
	SAFE_RELEASE(effect);
}

void Effect::CreateEffect()
{
	ID3DBlob* fxBlob = NULL;

	Effects::GetEffect(file, &fxBlob, &effect);

	effect->GetDesc(&effectDesc);
	for (UINT t = 0; t < effectDesc.Techniques; t++)
	{
		Technique technique;
		technique.ITechnique = effect->GetTechniqueByIndex(t);
		technique.ITechnique->GetDesc(&technique.Desc);
		technique.Name = String::ToWString(technique.Desc.Name);

		for (UINT p = 0; p < technique.Desc.Passes; p++)
		{
			Pass pass;
			pass.IPass = technique.ITechnique->GetPassByIndex(p);
			pass.IPass->GetDesc(&pass.Desc);
			pass.Name = String::ToWString(pass.Desc.Name);
			pass.IPass->GetVertexShaderDesc(&pass.PassVsDesc);
			pass.PassVsDesc.pShaderVariable->GetShaderDesc(pass.PassVsDesc.ShaderIndex, &pass.EffectVsDesc);

			for (UINT s = 0; s < pass.EffectVsDesc.NumInputSignatureEntries; s++)
			{
				D3D11_SIGNATURE_PARAMETER_DESC desc;

				//	GetInputSignatureElementDesc : 이전에 reflection이랑 같은 기능
				HRESULT hr = pass.PassVsDesc.pShaderVariable->GetInputSignatureElementDesc(pass.PassVsDesc.ShaderIndex, s, &desc);
				assert(SUCCEEDED(hr));

				pass.SignatureDescs.push_back(desc);
			}

			pass.InputLayout = CreateInputLayout(fxBlob, &pass.EffectVsDesc, pass.SignatureDescs);
			pass.StateBlock = initialStateBlock;

			technique.Passes.push_back(pass);
		}

		techniques.push_back(technique);
	}

	for (UINT i = 0; i < effectDesc.ConstantBuffers; i++)
	{
		EffectBuffer cbuffer;

		ID3DX11EffectConstantBuffer* iBuffer;
		iBuffer = effect->GetConstantBufferByIndex(i);

		D3DX11_EFFECT_VARIABLE_DESC vDesc;
		iBuffer->GetDesc(&vDesc);

		cbuffer.index = i;
		cbuffer.Buffer = iBuffer;
		cbuffer.Desc = vDesc;

		cBufferMap.insert(pair<string, EffectBuffer>(vDesc.Name, cbuffer));
	}

	//	GlobalVariables 해당 shader파일 내의 전역 변수들
	for (UINT i = 0; i < effectDesc.GlobalVariables; i++)
	{
		EffectVariable variable;

		ID3DX11EffectVariable* iVariable;
		iVariable = effect->GetVariableByIndex(i);

		D3DX11_EFFECT_VARIABLE_DESC vDesc;
		iVariable->GetDesc(&vDesc);

		variable.index = i;
		variable.Variable = iVariable;

		variableMap.insert(pair<string, EffectVariable>(vDesc.Name, variable));
	}

	//SAFE_RELEASE(fxBlob);
}

ID3D11InputLayout * Effect::CreateInputLayout(ID3DBlob * fxBlob, D3DX11_EFFECT_SHADER_DESC* effectVsDesc, vector<D3D11_SIGNATURE_PARAMETER_DESC>& params)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (D3D11_SIGNATURE_PARAMETER_DESC& paramDesc : params)
	{
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		string name = paramDesc.SemanticName;

		transform(name.begin(), name.end(), name.begin(), toupper);
		if (name == "POSITION")
			elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

		if (name == "INSTANCE")
		{
			elementDesc.InputSlot = 1;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;
		}

		inputLayoutDesc.push_back(elementDesc);
	}

	const void* pCode = effectVsDesc->pBytecode;
	UINT pCodeSize = effectVsDesc->BytecodeLength;

	if (inputLayoutDesc.size() > 0)
	{
		ID3D11InputLayout* inputLayout = NULL;
		HRESULT hr = D3D::GetDevice()->CreateInputLayout
		(
			&inputLayoutDesc[0]
			, inputLayoutDesc.size()
			, pCode
			, pCodeSize
			, &inputLayout
		);
		assert(SUCCEEDED(hr));

		return inputLayout;
	}

	return NULL;
}

void Effect::Pass::Draw(UINT vertexCount, UINT startVertexLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->Draw(vertexCount, startVertexLocation);
	}
	EndDraw();
}

void Effect::Pass::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}
	EndDraw();
}

void Effect::Pass::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}
	EndDraw();
}

void Effect::Pass::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startIndexLocation);
	}
	EndDraw();
}

void Effect::Pass::BeginDraw()
{
	IPass->ComputeStateBlockMask(&StateBlockMask);

	D3D::GetDC()->IASetInputLayout(InputLayout);
	IPass->Apply(0, D3D::GetDC());
}

void Effect::Pass::EndDraw()
{
	ID3D11ShaderResourceView * nullSRV[16] = { 0 };

	D3D::GetDC()->VSSetShaderResources(0, 16, nullSRV);
	D3D::GetDC()->GSSetShaderResources(0, 16, nullSRV);
	D3D::GetDC()->DSSetShaderResources(0, 16, nullSRV);
	D3D::GetDC()->HSSetShaderResources(0, 16, nullSRV);
	D3D::GetDC()->PSSetShaderResources(0, 16, nullSRV);
	D3D::GetDC()->CSSetShaderResources(0, 16, nullSRV);

	if (StateBlockMask.GS == 1)
		D3D::GetDC()->GSSetShader(NULL, NULL, 0);

	if (StateBlockMask.RSRasterizerState == 1)
		D3D::GetDC()->RSSetState(StateBlock->RSRasterizerState);

	if (StateBlockMask.OMDepthStencilState == 1)
		D3D::GetDC()->OMSetDepthStencilState(StateBlock->OMDepthStencilState, StateBlock->OMStencilRef);

	if (StateBlockMask.OMBlendState == 1)
		D3D::GetDC()->OMSetBlendState(StateBlock->OMBlendState, StateBlock->OMBlendFactor, StateBlock->OMSampleMask);

	D3D::GetDC()->HSSetShader(NULL, NULL, 0);
	D3D::GetDC()->DSSetShader(NULL, NULL, 0);
	D3D::GetDC()->GSSetShader(NULL, NULL, 0);
	D3D::GetDC()->RSSetState(NULL);
}

void Effect::Pass::Dispatch(UINT x, UINT y, UINT z)
{
	IPass->Apply(0, D3D::GetDC());
	D3D::GetDC()->Dispatch(x, y, z);

	ID3D11ShaderResourceView * null[1] = { 0 };
	D3D::GetDC()->CSSetShaderResources(0, 1, null);

	ID3D11UnorderedAccessView* nullUav[1] = { 0 };
	D3D::GetDC()->CSSetUnorderedAccessViews(0, 1, nullUav, NULL);

	D3D::GetDC()->CSSetShader(NULL, NULL, 0);
}

ID3DX11EffectVariable * Effect::Variable(string name)
{
	return variableMap[name].Variable;
}

ID3DX11EffectScalarVariable * Effect::AsScalar(string name)
{
	return variableMap[name].Variable->AsScalar();
}

ID3DX11EffectVectorVariable * Effect::AsVector(string name)
{
	return variableMap[name].Variable->AsVector();
}

ID3DX11EffectMatrixVariable * Effect::AsMatrix(string name)
{
	return variableMap[name].Variable->AsMatrix();
}

ID3DX11EffectStringVariable * Effect::AsString(string name)
{
	return variableMap[name].Variable->AsString();
}

ID3DX11EffectShaderResourceVariable * Effect::AsShaderResource(string name)
{
	return variableMap[name].Variable->AsShaderResource();
}

ID3DX11EffectRenderTargetViewVariable * Effect::AsRTV(string name)
{
	return variableMap[name].Variable->AsRenderTargetView();
}

ID3DX11EffectDepthStencilViewVariable * Effect::AsDSV(string name)
{
	return variableMap[name].Variable->AsDepthStencilView();
}

ID3DX11EffectConstantBuffer * Effect::AsConstantBuffer(string name)
{
	return cBufferMap[name].Buffer;
}

ID3DX11EffectShaderVariable * Effect::AsShader(string name)
{
	return variableMap[name].Variable->AsShader();
}

ID3DX11EffectBlendVariable * Effect::AsBlend(string name)
{
	return variableMap[name].Variable->AsBlend();
}

ID3DX11EffectDepthStencilVariable * Effect::AsDepthStencil(string name)
{
	return variableMap[name].Variable->AsDepthStencil();
}

ID3DX11EffectRasterizerVariable * Effect::AsRasterizer(string name)
{
	return variableMap[name].Variable->AsRasterizer();
}

ID3DX11EffectSamplerVariable * Effect::AsSampler(string name)
{
	return variableMap[name].Variable->AsSampler();
}

ID3DX11EffectUnorderedAccessViewVariable * Effect::AsUAV(string name)
{
	return variableMap[name].Variable->AsUnorderedAccessView();
}

void Effect::Technique::Draw(UINT pass, UINT vertexCount, UINT startVertexLocation)
{
	Passes[pass].Draw(vertexCount, startVertexLocation);
}

void Effect::Technique::DrawIndexed(UINT pass, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	Passes[pass].DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void Effect::Technique::DrawInstanced(UINT pass, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	Passes[pass].DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Effect::Technique::DrawIndexedInstanced(UINT pass, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	Passes[pass].DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void Effect::Technique::Dispatch(UINT pass, UINT x, UINT y, UINT z)
{
	Passes[pass].Dispatch(x, y, z);
}

void Effect::Draw(UINT technique, UINT pass, UINT vertexCount, UINT startVertexLocation)
{
	techniques[technique].Passes[pass].Draw(vertexCount, startVertexLocation);
}

void Effect::DrawIndexed(UINT technique, UINT pass, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	techniques[technique].Passes[pass].DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void Effect::DrawInstanced(UINT technique, UINT pass, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	techniques[technique].Passes[pass].DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Effect::DrawIndexedInstanced(UINT technique, UINT pass, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	techniques[technique].Passes[pass].DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void Effect::Dispatch(UINT technique, UINT pass, UINT x, UINT y, UINT z)
{
	techniques[technique].Passes[pass].Dispatch(x, y, z);
}

///////////////////////////////////////////////////////////////////////////////

unordered_map<wstring, Effects::EffectDesc> Effects::effects;

void Effects::Delete()
{
	for (Pair p : effects)
	{
		SAFE_RELEASE(p.second.blob);
		SAFE_RELEASE(p.second.effect);
	}
}

void Effects::GetEffect(wstring fileName, ID3DBlob** blob, ID3DX11Effect** effect)
{
	bool isFind = false;

	if (effects.count(fileName) < 1)
	{
		Pair p;

		// 못찾았을 경우.
		ID3DBlob* error;
#if 0
		INT flag = D3D10_SHADER_ENABLE_STRICTNESS;
#else
		INT flag = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
#endif

		//	D3D_COMPILE_STANDARD_FILE_INCLUDE : 같은 폴더에 있는 파일만 #include 가능
		//	NULL 은 #include 못함
		//	이제부터는 열우선이 아니라 행우선
		HRESULT hr = D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", flag, NULL, &p.second.blob, &error);
		if (FAILED(hr))
		{
			if (error != NULL)
			{
				string str = (const char *)error->GetBufferPointer();
				MessageBoxA(NULL, str.c_str(), "Shader Error", MB_OK);
			}
			assert(false);
		}

		hr = D3DX11CreateEffectFromMemory(p.second.blob->GetBufferPointer(), p.second.blob->GetBufferSize(), 0, D3D::GetDevice(), &p.second.effect);
		assert(SUCCEEDED(hr));

		p.first = fileName;

		effects.insert(p);

		*blob = p.second.blob;
		p.second.effect->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect);
	}
	else
	{
		EffectDesc desc = effects.at(fileName);

		*blob = desc.blob;
		desc.effect->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect);
	}
}