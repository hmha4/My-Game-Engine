#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  Constant Buffers
//  --------------------------------------------------------------------------- //


//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //
RasterizerState FillModeOn
{
    FillMode = Wireframe;
};


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
    
    return output; // 반환값이 픽셀의 위치
}

//  --------------------------------------------------------------------------- //
//  Pixel Shader
//  --------------------------------------------------------------------------- //
float4 PS_Red(VertexOutput input) : SV_TARGET
{
    return float4(1, 0, 0, 1);
}

float4 PS_Green(VertexOutput input) : SV_TARGET
{
    return float4(0, 1, 0, 1);
}

//  --------------------------------------------------------------------------- //
//  Technique & Pass
//  --------------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Red()));

        SetRasterizerState(FillModeOn);
    }

    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Green()));
    }
}