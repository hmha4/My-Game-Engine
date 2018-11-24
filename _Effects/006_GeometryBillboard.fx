#include "000_header.fx"

// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
//  States
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Center : POSITION0;
    float2 Size : SIZE0;
};

VertexOutput VS(VertexSize input)
{
    VertexOutput output;

    output.Center = input.Position;
    output.Size = input.Size;

    return output;
}

// --------------------------------------------------------------------- //
//  Geometry Shader
// --------------------------------------------------------------------- //

struct GeometryOutput
{
    float4 Position : SV_POSITION;
    float3 wPosition : POSITION1;
    float3 LookDir : DIR0;
    float2 Uv : UV0;
    uint PrimitiveId : PRIMITIVEID0;
};

static const float2 TexCoord[4] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
};


///////////////////////////////////////////////////////////////////////////////
//  Geometry Shader(GS)
//  ������Ʈ�� ���̴��� �ϳ��� �������� �������� ������ ���� �� �ִ�.
//  ��, �ϳ��� �ﰢ������ 4���� �ﰢ���� ����� �ִ�.
//  input�� output�� ��ȣ���谡 ����.

//  GS���� 3���� ������ ���ؽ� ��Ʈ���� �ִ�.
//  PointStream, LineStream, TriangleStream
//  LineStream�� TriangleStream�� ���� ��쿡�� output topology�� �׻� strip�̴�.

//  GS�Լ��� �Ű������� input ���˰� output ��Ʈ�� Ÿ���� �����ؾ� �Ѵ�.
//  �׸��� �ִ�� ��ȯ�� ���ؽ��� ������ �����ؾ� �Ѵ�.
///////////////////////////////////////////////////////////////////////////////

[maxvertexcount(4)] //  �ִ� �Ѱ� ���� ���ؽ� ����
//  input[n] : �Ѱ� ���� ���ؽ� ����
//  stream : ��µ� ��(TriangleStream : �ﰢ�� ������), �ȼ� ���̴��� ���� �ڷ���
void GS(point VertexOutput input[1], uint PrimitiveId : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    //  �������κ��� 4���� ������ ���ϱ� ���� Size�� ������ ����
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;

    //  �������κ��� 4���� ������ ���ϱ� ���� up, forward, right ���͸� ����
    float3 look = ViewPosition - input[0].Center.xyz;   //  ī�޶� �ٶ󺸵��� ���
    look.y = 0.0f;
    look = normalize(look);

    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, look));
    
    //  up�� right, halfWidth, halfHeight�� ����ؼ� 4���� ������ ����
    float4 v[4];
    v[0] = float4(input[0].Center.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Center.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Center.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Center.xyz - halfWidth * right + halfHeight * up, 1.0f);

    GeometryOutput output;

    //  GS�� VS�� ���������� 3D ������ ����ϱ� ���ؼ���
    //  WVP��ȯ�� �ʿ��ϴ�.
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        output.Position = mul(v[i], World);
        output.Position = mul(output.Position, View);
        output.Position = mul(output.Position, Projection);

        output.wPosition = mul(v[i], World).xyz;

        output.Uv = TexCoord[i];
        output.LookDir = look;
        output.PrimitiveId = PrimitiveId;

        stream.Append(output);
    }
}


// --------------------------------------------------------------------- //
//  Pixel Buffer
// --------------------------------------------------------------------- //
Texture2DArray Map;
SamplerState Sampler;

float4 PS(GeometryOutput input) : SV_TARGET
{
    float3 uvw = float3(input.Uv, input.PrimitiveId % 6);

    float4 color = Map.Sample(Sampler, uvw);
    clip(color.a - 0.5f);

    return color;
}

// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}