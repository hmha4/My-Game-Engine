#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Global Variables
// --------------------------------------------------------------------- //
cbuffer CB_Terrain
{
    float4 FogColor;
    float FogStart;
    float FogRange;

    float MinDistance;
    float MaxDistance;
    float MinTessellation;
    float MaxTessellation;

    float TexelCellSpaceU;
    float TexelCellSpaceV;
    float WorldCellSpace;

    float2 TexScale = 66.0f;
    float CB_Terrain_Padding;

    float4 WorldFrustumPlanes[6];
};


// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //

Texture2D HeightMap;
SamplerState HeightMapSampler
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;

    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct VertexInput
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundsY : BoundsY0;
};

struct VertexOutput
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundsY : BoundsY0;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    output.Position.y = HeightMap.SampleLevel(HeightMapSampler, input.Uv, 0).r;

    output.Uv = input.Uv;
    output.BoundsY = input.BoundsY;

    //  프랙탈 알고리즘

    return output;
}

// --------------------------------------------------------------------- //
//  Hull Shader
// --------------------------------------------------------------------- //
float CalcTessFactor(float3 position)
{
    float d = distance(position, ViewPosition);
    float s = saturate((d - MinDistance) / (MaxDistance - MinDistance));

    return pow(2, lerp(MaxTessellation, MinTessellation, s));
}

bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
    float3 n = abs(plane.xyz);
	
	// This is always positive.
    float r = dot(extents, n);
	
	// signed distance from center point to plane.
    float s = dot(float4(center, 1.0f), plane);
	
	// If the center point of the box is a distance of e or more behind the
	// plane (in which case s is negative since it is behind the plane),
	// then the box is completely in the negative half space of the plane.
    return (s + r) < 0.0f;
}

bool AabbOutsideFrustumTest(float3 center, float3 extents)
{
    [unroll]
    for (int i = 0; i < 6; ++i)
    {
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum.
        [flatten]
        if (AabbBehindPlaneTest(center, extents, WorldFrustumPlanes[i]))
        {
            return true;
        }
    }
	
    return false;
}

struct ConstantOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsidetessFactor;
};

ConstantOutput HS_Constant(InputPatch<VertexOutput, 4> input, uint patchId : SV_PrimitiveId)
{
    ConstantOutput output;

    float minY = input[0].BoundsY.x;
    float maxY = input[0].BoundsY.y;

    float3 vMin = float3(input[2].Position.x, minY, input[2].Position.z);
    float3 vMax = float3(input[1].Position.x, maxY, input[1].Position.z);

    float3 boxCenter = (vMin + vMax) * 0.5f;
    float3 boxExtents = (vMax - vMin) * 0.5f;

    if (AabbOutsideFrustumTest(boxCenter, boxExtents))
    {
        output.Edges[0] = 0.0f;
        output.Edges[1] = 0.0f;
        output.Edges[2] = 0.0f;
        output.Edges[3] = 0.0f;

        output.Inside[0] = 0.0f;
        output.Inside[1] = 0.0f;
        
        return output;
    }

    float3 e0 = (input[0].Position + input[2].Position).xyz * 0.5f;
    float3 e1 = (input[0].Position + input[1].Position).xyz * 0.5f;
    float3 e2 = (input[1].Position + input[3].Position).xyz * 0.5f;
    float3 e3 = (input[2].Position + input[3].Position).xyz * 0.5f;
    float3 c = (input[0].Position + input[1].Position + input[2].Position + input[3].Position).xyz * 0.25f;

    output.Edges[0] = CalcTessFactor(e0);
    output.Edges[1] = CalcTessFactor(e1);
    output.Edges[2] = CalcTessFactor(e2);
    output.Edges[3] = CalcTessFactor(e3);

    output.Inside[0] = CalcTessFactor(c);
    output.Inside[1] = output.Inside[0];

    return output;
}

struct HullOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

//  line
//  line_adj
//  tri
//  tri_adj
[domain("quad")]
//  integer : 무조건 올림 변환
//  fractional_odd : 홀수 일 때 선 분할
//  fractional_even : 짝수 일 때 선 분할
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64.0f)] //  사용해도되고 안해도 되지만 사용하면 속도가 조금 빨라짐
HullOutput HS(InputPatch<VertexOutput, 4> input, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveId)
{
    HullOutput output;
    output.Position = input[pointID].Position;
    output.Uv = input[pointID].Uv;

    return output;
}

// --------------------------------------------------------------------- //
//  Domain Shader
// --------------------------------------------------------------------- //
struct DomainOutput
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float2 Uv : Uv0;
    float2 TiledUv : Uv1;
};

[domain("quad")]
DomainOutput DS(ConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HullOutput, 4> patch)
{
    DomainOutput output;
    
    float3 p1 = lerp(patch[0].Position.xyz, patch[1].Position.xyz, uv.x).xyz;
    float3 p2 = lerp(patch[2].Position.xyz, patch[3].Position.xyz, uv.x).xyz;
    output.wPosition = lerp(p1, p2, uv.y);

    float2 uv1 = lerp(patch[0].Uv.xy, patch[1].Uv.xy, uv.x);
    float2 uv2 = lerp(patch[2].Uv.xy, patch[3].Uv.xy, uv.x);
    output.Uv = lerp(uv1, uv2, uv.y);
    
    output.wPosition.y = HeightMap.SampleLevel(HeightMapSampler, output.Uv, 0).r;

    output.Position = mul(float4(output.wPosition, 1), World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    // Tile layer textures over terrain.
    output.TiledUv = output.Uv * TexScale;

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2DArray LayerMapArray;
Texture2D BlendMap;
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(DomainOutput input, uniform bool fogEnabled) : SV_TARGET
{
    float2 left = input.Uv + float2(-TexelCellSpaceU, 0.0f);
    float2 right = input.Uv + float2(TexelCellSpaceU, 0.0f);
    float2 bottom = input.Uv + float2(0.0f, TexelCellSpaceV);
    float2 top = input.Uv + float2(0.0f, -TexelCellSpaceV);

    float leftY = HeightMap.SampleLevel(HeightMapSampler, left, 0).r;
    float rightY = HeightMap.SampleLevel(HeightMapSampler, right, 0).r;
    float bottomY = HeightMap.SampleLevel(HeightMapSampler, bottom, 0).r;
    float topY = HeightMap.SampleLevel(HeightMapSampler, top, 0).r;

    float3 tangent = normalize(float3(WorldCellSpace * 2.0f, rightY - leftY, 0.0f));
    float3 biTangent = normalize(float3(0.0f, bottomY - topY, WorldCellSpace * -2.0f));
    float3 normalW = cross(tangent, biTangent);
    
    float3 eye = ViewPosition - input.wPosition;
    float distanceToEye = length(eye);
    eye /= distanceToEye;
    
    // Sample layers in texture array.
    float4 c0 = LayerMapArray.Sample(Sampler, float3(input.TiledUv, 0));
    float4 c1 = LayerMapArray.Sample(Sampler, float3(input.TiledUv, 1));
    float4 c2 = LayerMapArray.Sample(Sampler, float3(input.TiledUv, 2));
    float4 c3 = LayerMapArray.Sample(Sampler, float3(input.TiledUv, 3));
    float4 c4 = LayerMapArray.Sample(Sampler, float3(input.TiledUv, 4));

    // Sample the blend map.
    float4 t = BlendMap.Sample(Sampler, input.Uv);
    
    // Blend the layers on top of each other.
    float4 color = c0;
    color = lerp(color, c1, t.r);
    color = lerp(color, c2, t.g);
    color = lerp(color, c3, t.b);
    color = lerp(color, c4, t.a);

    [flatten]
    if (fogEnabled == true)
    {
        float fogFactor = saturate((distanceToEye - FogStart) / FogRange);

        color = lerp(color, FogColor, fogFactor);
    }
    
    return color;
}

// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
RasterizerState FillMode
{
    FillMode = Wireframe;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(NULL);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS(true)));
    }

    pass P1
    {
        SetRasterizerState(FillMode);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS(true)));
    }
}