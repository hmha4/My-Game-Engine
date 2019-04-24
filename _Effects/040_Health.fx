// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //
matrix World;
matrix View;
matrix Projection;

float AlphaValue;

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //

struct VertexTexture
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
};

VertexOutput VS(VertexTexture input)
{
    //  W : 동촤 -> 0 : 방향 , 1 : 위치
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    //  NDC 공간 - FVF_RHWXYZ
    //output.Position = input.Position;

    output.Uv = input.Uv;
    return output; // 반환값이 픽셀의 위치
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Map;
Texture2D MapSecond;
float UVx = -1;
float UVy = -1;
float Time;
float4 Color;

float4 PS(VertexOutput input) : SV_TARGET
{
     //  W : 동촤 -> NDC화면 공간에 대한 비율
    float4 color = 0;

    if (UVy != -1)
    {
        if (input.Uv.y >= 1.0f - UVy)
        {
            color = Map.Sample(Sampler, input.Uv);
            float2 uv = float2(input.Uv.x, (input.Uv.y + Time * 0.2f) % 1.0f);
            float4 secondary = MapSecond.Sample(Sampler, uv);

            color = float4(color.r * Color.r, color.g * Color.g, color.b * Color.b, color.a);
            color.rgb *= secondary.rgb;
        }
    }
    
    if (UVx != -1)
    {
        if (input.Uv.x <= UVx)
        {
            color = Map.Sample(Sampler, input.Uv);

            float x = input.Uv.x - Time * 0.2f;
            x = abs(x) % 1.0f;
            float2 uv = float2(x, input.Uv.y);
            float4 secondary = MapSecond.Sample(Sampler, uv);

            color = float4(color.r * Color.r, color.g * Color.g, color.b * Color.b, color.a);
            color.rgb *= secondary.rgb;
        }
    }
    //color.a = AlphaValue;

    //color.rgb += float3(1, 0, 0);

    

    return color;
}

// --------------------------------------------------------------------- //
//  States
// --------------------------------------------------------------------- //
DepthStencilState Depth
{
    DepthEnable = false;
};
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
// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetDepthStencilState(Depth, 0);
        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}