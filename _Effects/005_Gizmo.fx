#include "000_Header.fx"

float4 Color = { 1, 0, 0, 1 };
bool IsPick = false;

//=====================================================//
// States
//=====================================================//
//RasterizerState CullModeOn
//{
//    CullMode = None;
//};

//DepthStencilState DepthOff
//{
//    DepthEnable = false;
//};

//BlendState BlendOn
//{
//    BlendEnable=true;
//};

//=====================================================//
// VertexShader
//=====================================================//

struct VertexOutput
{
    float4 Position : SV_POSITION; 
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
};

VertexOutput VS(VertexTextureNormal input)
{
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Uv = input.Uv;
    output.Normal = input.Normal;

    return output; // 반환값이 픽셀의 위치
}


//=====================================================//
// PixelShader
//=====================================================//
float4 PS(VertexOutput input) : SV_TARGET
{
    float4 color = Color;

    if (IsPick==false)
        color.a = 0.0f;
    
    if(IsPick==true)
        color.a = 0.8f;

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
        
        //SetRasterizerState(CullModeOn); 
        //SetDepthStencilState(DepthOff);
    }
    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

       // SetRasterizerState(CullModeOn);
        //SetDepthStencilState(DepthOff);
        //SetBlendState(BlendOn);
    }
}