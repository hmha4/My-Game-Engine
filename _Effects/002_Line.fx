#include "000_Header.fx"

float4 Color = { 1, 0, 0, 1 };

//=====================================================//
// States
//=====================================================//


//=====================================================//
// VertexShader
//=====================================================//

DepthStencilState DepthOff
{
    DepthEnable = false;
};


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


//=====================================================//
// PixelShader
//=====================================================//
float4 PS(VertexOutput input) : SV_TARGET
{
    return Color;
}


//=====================================================//
// technique
//=====================================================//
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        SetDepthStencilState(DepthOff, 0);
    }
}