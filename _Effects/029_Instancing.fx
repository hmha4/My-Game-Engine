#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct Test
{
    float3 Position;
};
StructuredBuffer<Test> Input;

struct VertexInput
{
    float4  Position : Position0;
    uint Id : SV_VertexID;  //  CS 처럼 DispatchID 역할
};

struct VertexOutput
{
    float4 Position : SV_Position0;
    uint Id : VertexID0;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    output.Position.xyz = input.Position.xyz + Input[input.Id].Position;
    output.Position.w = 1.0f;

    output.Id = input.Id;

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2DArray Map;
SamplerState Sampler;

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 color = 0;
    if(input.Id % 2 == 0)
        color = float4(1, 0, 0, 1);
    else
        color = float4(0, 0, 1, 1);

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