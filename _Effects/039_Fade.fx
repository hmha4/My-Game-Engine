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
    //  W : ���� -> 0 : ���� , 1 : ��ġ
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    //  NDC ���� - FVF_RHWXYZ
    //output.Position = input.Position;

    output.Uv = input.Uv;
    return output; // ��ȯ���� �ȼ��� ��ġ
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Map;

float4 PS(VertexOutput input) : SV_TARGET
{
     //  W : ���� -> NDCȭ�� ������ ���� ����
    float4 color = Map.Sample(Sampler, input.Uv);
    color.a = AlphaValue;

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
    AlphaToCoverageEnable = false;

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