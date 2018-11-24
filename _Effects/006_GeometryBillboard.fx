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
//  지오메트리 셰이더는 하나의 정점으로 여러개의 정점을 만들 수 있다.
//  즉, 하나의 삼각형으로 4개의 삼각형도 만들수 있다.
//  input과 output은 상호관계가 없다.

//  GS에는 3가지 종류의 버텍스 스트림이 있다.
//  PointStream, LineStream, TriangleStream
//  LineStream과 TriangleStream과 같은 경우에는 output topology가 항상 strip이다.

//  GS함수는 매개변수로 input 포맷과 output 스트림 타입을 지정해야 한다.
//  그리고 최대로 반환할 버텍스의 갯수를 지정해야 한다.
///////////////////////////////////////////////////////////////////////////////

[maxvertexcount(4)] //  최대 넘겨 보낼 버텍스 갯수
//  input[n] : 넘겨 받을 버텍스 개수
//  stream : 출력될 놈(TriangleStream : 삼각형 단위로), 픽셀 셰이더로 나갈 자료형
void GS(point VertexOutput input[1], uint PrimitiveId : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    //  중점으로부터 4개의 정점을 구하기 위해 Size를 반으로 나눔
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;

    //  중점으로부터 4개의 정점을 구하기 위해 up, forward, right 벡터를 구함
    float3 look = ViewPosition - input[0].Center.xyz;   //  카메라를 바라보도록 계산
    look.y = 0.0f;
    look = normalize(look);

    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, look));
    
    //  up과 right, halfWidth, halfHeight를 사용해서 4개의 정점을 구함
    float4 v[4];
    v[0] = float4(input[0].Center.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Center.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Center.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Center.xyz - halfWidth * right + halfHeight * up, 1.0f);

    GeometryOutput output;

    //  GS도 VS와 마찬가지로 3D 공간상에 출력하기 위해서는
    //  WVP변환이 필요하다.
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