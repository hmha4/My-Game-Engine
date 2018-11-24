#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  Constant Buffers
//  --------------------------------------------------------------------------- //
float4 Color = float4(1, 1, 1, 1);

//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //



//  --------------------------------------------------------------------------- //
//  Vertex Shader
//  --------------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : SV_POSITION;
};

VertexOutput VS(Vertex input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    return output;
}

//  --------------------------------------------------------------------------- //
//  Pixel Shader
//  --------------------------------------------------------------------------- //

float4 PS(VertexOutput input) : SV_TARGET
{
    return Color;
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