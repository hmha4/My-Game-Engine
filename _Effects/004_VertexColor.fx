#include "000_Header.fx"

struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float4 Color : COLOR0;
};


VertexOutput VS(VertexColor input)
{
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Color = input.Color;

    return output; // ��ȯ���� �ȼ��� ��ġ
}

float4 PS(VertexOutput input) : SV_TARGET
{
	//���� float4 = > r,g,b,a
    return input.Color;
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
}