#include "000_Header.fx"

// ---------------------------------------------------------------------------- //
// States
// ---------------------------------------------------------------------------- //


// ---------------------------------------------------------------------------- //
// VertexShader
// ---------------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 Uv : UV0;
};

VertexOutput VS(VertexTextureNormal input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = mul(input.Normal, (float3x3) World);
    output.Uv = input.Uv;
    
    return output;
}

// ---------------------------------------------------------------------------- //
// PixelShader
// ---------------------------------------------------------------------------- //
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 diffuse = DiffuseMap.Sample(Sampler, input.Uv);
    float3 normal = normalize(input.Normal);
    float NdotL = dot(normal, -LightDirection);

    return diffuse * NdotL;
}

// ---------------------------------------------------------------------------- //
// Technique & Pass
// ---------------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        //  필요없는 경우 : SetVertexShader(NULL)
        SetVertexShader(CompileShader(vs_5_0, VS()));
        //SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}