#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct VertexInput
{
    float4  Position : POSITION0;
    float2  Uv : UV0;
    matrix  World : INSTANCE0;
    uint    TextureId : INSTANCE4;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
    uint TextureId : TEXTUREID0;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    output.Position = mul(input.Position, input.World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Uv = input.Uv;
    output.TextureId = input.TextureId;

    return output;

}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2DArray Map;
SamplerState Sampler;

float4 PS(VertexOutput input) : SV_TARGET
{
    float3 uvw = float3(input.Uv, input.TextureId);

    float4 color = Map.Sample(Sampler, uvw);

    return color;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}