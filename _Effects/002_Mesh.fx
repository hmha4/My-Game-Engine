#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  Constant Buffers
//  --------------------------------------------------------------------------- //


//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //



//  --------------------------------------------------------------------------- //
//  Vertex Shader
//  --------------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

VertexOutput VS(VertexTextureNormalTangent input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = mul(input.Normal, (float3x3) World);
    output.Tangent = mul(input.Tangent, (float3x3) World);
    
    output.Uv = input.Uv;
    
    return output;
}

//  --------------------------------------------------------------------------- //
//  Pixel Shader
//  --------------------------------------------------------------------------- //
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    return DiffuseMap.Sample(Sampler, input.Uv);
}

//  --------------------------------------------------------------------------- //
//  Technique & Pass
//  --------------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}