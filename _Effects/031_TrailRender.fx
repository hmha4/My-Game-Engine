#include "000_Header.fx"
//=====================================================//
// Globals
//=====================================================//
float SliceAmount = 0.0f;
float BurnSize = 0.15f;
float4 BurnColor = float4(1, 1, 1, 1);
float EmissionAmount = 2.0f;

//=====================================================//
// States
//=====================================================//
BlendState BlendOn
{
    BlendEnable[0] = true;
    BlendOp[0] = Add;
    BlendOpAlpha[0] = Add;
    DestBlend[0] = Inv_Src_Alpha;
    DestBlendAlpha[0] = Zero;
    SrcBlend[0] = Src_Alpha;
    SrcBlendAlpha[0] = One;
    RenderTargetWriteMask[0] = 1 | 2 | 4 | 8; // Write all colors R | G | B | A
};

RasterizerState CullModeOff
{
    CullMode = None;
    //FillMode = Wireframe;
};



//=====================================================//
// VertexShader
//=====================================================//


struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 wPosition : Position1;

    float3 Normal : Normal0;
    float2 Uv : UV0;
};

VertexOutput VS(VertexTextureNormal input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.wPosition = output.Position;

    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Uv = input.Uv;
    output.Normal = input.Normal;

    return output; // 반환값이 픽셀의 위치
}


//=====================================================//
// PixelShader
//=====================================================//

Texture2D TrailMap;
Texture2D AlphaMap;
SamplerState TrailSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D SliceGuide;
Texture2D BurnMap;
Texture2D BurnRamp;

float4 PS(VertexOutput input) : SV_TARGET
{
  
    float4 diffuse = TrailMap.Sample(TrailSampler, input.Uv);
    float4 alpha = AlphaMap.Sample(TrailSampler, input.Uv);
    float4 color = float4(diffuse.rgb, alpha.r);

    half test = SliceGuide.Sample(TrailSampler, input.Uv).rgb - SliceAmount;
    clip(test);

    //float4 temp;
    //if (test < BurnSize && SliceAmount > 0)
    //{
    //    temp = BurnRamp.Sample(TrailSampler, float2(test * (1 / BurnSize), 0)) * BurnColor * EmissionAmount;
    //}
    
    return color/* * 2*/;
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

        SetRasterizerState(CullModeOff);
        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}