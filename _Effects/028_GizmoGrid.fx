#include "000_Header.fx"

struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float4 wPosition : Position1;
    float4 Color : COLOR0;
};


VertexOutput VS(VertexColor input)
{
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.wPosition = output.Position;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Color = input.Color;

    return output; // 반환값이 픽셀의 위치
}

float4 PS(VertexOutput input) : SV_TARGET
{
	//색은 float4 = > r,g,b,a
    float3 toEye = ViewPosition - input.wPosition.xyz;
    float dist = length(toEye);
    dist = clamp(dist, 0, 500);

    float value = smoothstep(0, 500, dist);

    float4 color = input.Color;
    color.a = (1 - value);

    return color;
}

BlendState Blend
{
    AlphaToCoverageEnable = true;

    BlendEnable[0] = true;
    RenderTargetWriteMask[0] = 1 | 2 | 4 | 8; // Write all colors R | G | B | A

    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
};


//=====================================================//
// technique
//=====================================================//
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}