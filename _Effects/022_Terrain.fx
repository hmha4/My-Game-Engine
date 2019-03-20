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

    int TexScale;
    float CB_Terrain_Padding;

    float4 WorldFrustumPlanes[6];
};

float2 HeightMapSize; // HeightMap 가로/세로 크기
float2 TerrainSize; // 지형맵 버텍스 실제 위치의 크기

float MaxHeight;
float4 BlendPositionRate;
float4 ActiveBlend;

//브러쉬 타입
int Type;
float3 Location;
int Range;
float3 Color;

//터레인디테일 타입
float DetailValue = 0.999;
float DetailIntensity = 1;

//터레인라인 타입
int LineType;
float3 LineColor;
int Spacing; // 한칸의 넓이
float Thickness;

float3 BrushColor(float3 location)
{
    //location은 버텍스위치
    //Location은 지형의 피킹 위치
    if (Type == 0)
        return float3(0, 0, 0);
    //사각형일때
    if (Type == 1 || Type == 4)
    {
        if ((location.x >= (Location.x - Range)) &&
            (location.x <= (Location.x + Range)) &&
            (location.z >= (Location.z - Range)) &&
            (location.z <= (Location.z + Range)))
        {
            return Color;
        }
    }
    //원일때
    if (Type == 2 || Type == 3)
    {
        float dx = location.x - Location.x;
        float dy = location.z - Location.z;

        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= Range)
            return Color;
    }

    return float3(0, 0, 0);
}

float3 Line(float3 oPosition)
{
    //Line Drawing
    [branch] // 둘다실행하고 하나만 선택함 if else if사용가능
    if (LineType == 1)
    {
        float2 grid = float2(0, 0);
        grid.x = frac(oPosition.x / (float) Spacing);
        grid.y = frac(oPosition.z / (float) Spacing);

        [flatten]
        if (grid.x < Thickness || grid.y < Thickness)
            return LineColor;
    }
    else if (LineType == 2)
    {
        float2 grid = oPosition.xz / (float) Spacing;

        //끝지점을 맞추는작업(한칸의 범위)
        float2 range = abs(frac(grid - 0.5f) - 0.5f);
        
        //편미분(fwidth) 
        //월드상에선 xz가 스크린상에선 xy로 나타내지는것에
        //기울기를 구하기위해 편미분 사용
        float2 speed = fwidth(range);

        float2 pixel = range / speed;
        float weights = saturate(min(pixel.x, pixel.y) - Thickness);

        return lerp(LineColor, float3(0, 0, 0), weights);
    }

    return float3(0, 0, 0);
}
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
    float4 Position : POSITION0;
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

ConstantOutput HS_Constant_Depth(InputPatch<VertexOutput, 4> input, uint patchId : SV_PrimitiveId)
{
    ConstantOutput output;

    float minY = input[0].BoundsY.x;
    float maxY = input[0].BoundsY.y;

    float3 vMin = float3(input[2].Position.x, minY, input[2].Position.z);
    float3 vMax = float3(input[1].Position.x, maxY, input[1].Position.z);

    float3 boxCenter = (vMin + vMax) * 0.5f;
    float3 boxExtents = (vMax - vMin) * 0.5f;
    

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

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant_Depth")]
[maxtessfactor(64.0f)] //  사용해도되고 안해도 되지만 사용하면 속도가 조금 빨라짐
HullOutput HS_Depth(InputPatch<VertexOutput, 4> input, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveId)
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
    float4 pPosition : Position2;
    float2 Uv : Uv0;
    float2 TiledUv : Uv1;
    float3 BrushColor : BrushColor0;
    float4 ShadowPos : Uv2;
};

[domain("quad")]
DomainOutput DS(ConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HullOutput, 4> patch)
{
    DomainOutput output;
    
    float3 p1 = lerp(patch[0].Position.xyz, patch[1].Position.xyz, uv.x).xyz;
    float3 p2 = lerp(patch[2].Position.xyz, patch[3].Position.xyz, uv.x).xyz;
    
    float2 uv1 = lerp(patch[0].Uv.xy, patch[1].Uv.xy, uv.x);
    float2 uv2 = lerp(patch[2].Uv.xy, patch[3].Uv.xy, uv.x);
    output.Uv = lerp(uv1, uv2, uv.y);

    output.wPosition = lerp(p1, p2, uv.y);
    output.wPosition.y = HeightMap.SampleLevel(HeightMapSampler, output.Uv, 0).r;

    output.wPosition = mul(float4(output.wPosition, 1), World);
    output.ShadowPos = mul(float4(output.wPosition, 1), ShadowTransform);
    output.BrushColor = BrushColor(output.wPosition);

    output.Position = mul(float4(output.wPosition, 1), View);
    output.Position = mul(output.Position, Projection);
    
    output.pPosition = output.Position;
    // Tile layer textures over terrain.
    output.TiledUv = output.Uv * TexScale;

    return output;
}

[domain("quad")]
DomainOutput DS_Depth(ConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HullOutput, 4> patch)
{
    DomainOutput output;
    
    float3 p1 = lerp(patch[0].Position.xyz, patch[1].Position.xyz, uv.x).xyz;
    float3 p2 = lerp(patch[2].Position.xyz, patch[3].Position.xyz, uv.x).xyz;
    
    float2 uv1 = lerp(patch[0].Uv.xy, patch[1].Uv.xy, uv.x);
    float2 uv2 = lerp(patch[2].Uv.xy, patch[3].Uv.xy, uv.x);
    output.Uv = lerp(uv1, uv2, uv.y);

    output.wPosition = lerp(p1, p2, uv.y);
    output.wPosition.y = HeightMap.SampleLevel(HeightMapSampler, output.Uv, 0).r;

    output.wPosition = mul(float4(output.wPosition, 1), World);
    output.ShadowPos = mul(float4(output.wPosition, 1), ShadowTransform);
    output.BrushColor = BrushColor(output.wPosition);

    output.Position = mul(float4(output.wPosition, 1), LightView);
    output.Position = mul(output.Position, LightProjection);
    
    output.pPosition = output.Position;
    // Tile layer textures over terrain.
    output.TiledUv = output.Uv * TexScale;

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2DArray LayerMapArray;
Texture2D BlendMap;


bool IsHovering = false;
float2 PickPosition = float2(0, 0);
float2 BrushRate = float2(50, 50);

SamplerState TrilinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

struct PixelTargetOutput
{
    float4 tColor : SV_TARGET0;
    float4 pColor : SV_TARGET1;
};

float4 BlendLerpHelper(float4 fColor, float4 sColor, float wPosRate, float4 BPR, float minusRate, float plusRate)
{
    float4 rColor = fColor;
    
    rColor = (BPR < 0.01f && BPR > -0.001f) ? lerp(fColor, sColor, saturate((0.01f - wPosRate) / (0.01f + plusRate - minusRate))) : lerp(fColor, sColor, saturate((wPosRate - (BPR - minusRate)) / ((BPR + plusRate) - (BPR - minusRate))));

    return rColor;
}

float4 BlendLerpHelper2(float4 fColor, float4 sColor, float wPosRate, float4 BPR, float minusRate, float plusRate)
{
    float4 rColor = 0;

    rColor = lerp(fColor, sColor, saturate((wPosRate - (BPR - minusRate)) / ((BPR + plusRate) - (BPR - minusRate))));

    rColor = (BPR < 0.01f && wPosRate < 0.01f) ? sColor : rColor;

    return rColor;
}

float4 BrushHelper(float4 color, float2 uv, Texture2D brushTexture)
{
    float4 rColor = color;

    float2 mousePos = PickPosition / HeightMapSize;
    mousePos.y = 1.0f - mousePos.y; // 픽포지션 변환작업으로 반전되어있어서 다시 되돌려줘야함.

    float2 pix1uv = 1.0f / TerrainSize;
    float2 brushAreaMinUv = mousePos - pix1uv * (BrushRate * 0.5f);
    float2 brushAreaMaxUv = mousePos + pix1uv * (BrushRate * 0.5f);

    if (
        (uv.x > brushAreaMinUv.x) && (uv.x < brushAreaMaxUv.x)
        && (uv.y > brushAreaMinUv.y) && (uv.y < brushAreaMaxUv.y)
        )
    {
        float2 brushUv = (uv - brushAreaMinUv) / (brushAreaMaxUv - brushAreaMinUv);
        float4 brushColor = brushTexture.Sample(TrilinearSampler, brushUv);

#if 0
        rColor.xyz = brushColor.a > 0.2 ? brushColor.xyz : rColor.xyz;
#else
        brushColor.a = (brushColor.r > 0.99 && brushColor.g < 0.01f && brushColor.b > 0.99) ? 0 : brushColor.a;
        rColor.xyz = brushColor.a > 0.01f ? float3(0, 1, 0) : rColor.xyz;
#endif

    }

    return rColor;
}

PixelTargetOutput PS(DomainOutput input, uniform bool fogEnabled) : SV_TARGET
{
    PixelTargetOutput output;

    float2 left = input.Uv + float2(-TexelCellSpaceU, 0.0f);
    float2 right = input.Uv + float2(TexelCellSpaceU, 0.0f);
    float2 top = input.Uv + float2(0.0f, -TexelCellSpaceV);
    float2 bottom = input.Uv + float2(0.0f, TexelCellSpaceV);

    float leftY = HeightMap.SampleLevel(HeightMapSampler, left, 0).r;
    float rightY = HeightMap.SampleLevel(HeightMapSampler, right, 0).r;
    float topY = HeightMap.SampleLevel(HeightMapSampler, top, 0).r;
    float bottomY = HeightMap.SampleLevel(HeightMapSampler, bottom, 0).r;

    float3 tangent = normalize(float3(WorldCellSpace * 2.0f, rightY - leftY, 0.0f));
    float3 biTangent = normalize(float3(0.0f, bottomY - topY, WorldCellSpace * -2.0f));
    float3 normalW = cross(tangent, biTangent);
    
    float3 eye = ViewPosition - input.wPosition;
    float distanceToEye = length(eye);
    eye /= distanceToEye;

    float depth = input.pPosition.z / input.pPosition.w;
    float2 zValue = float2(1, 1);
    
    [branch]
    if (depth > DetailValue) //0.999
    {
        zValue = input.TiledUv * 0.25f;
    }
    else
    {
        zValue = input.TiledUv;
    }

    //======================터레인자체==========================//
    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);
    
    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;

    float4 diffuseColor = DiffuseMap.Sample(Sampler, zValue);
    float gamma = 2.2f;
    float3 power = 1.0f / gamma;
    diffuseColor.rgb = pow(diffuseColor.rgb, gamma);

    float4 color = diffuseColor;

    float4 specularColor = SpecularMap.Sample(Sampler, zValue);
    float sFactor = dot(specularColor, 1.0f);

    float4 detailMap = DetailMap.Sample(Sampler, zValue);

    float4 normalMap = NormalMap.Sample(Sampler, zValue);
    float3 normalValue;


    if (depth <= DetailValue)
    {
        if (normalMap.r != 0)
        {
            normalValue = NormalSampleToWorldSpace(normalMap.xyz, normalW, tangent);
            NormalMapping(color, normalMap, normalValue, tangent);
        }
        else
            normalValue = normalW;
    }
 
    if (sFactor > 0)
    {
        float4 spec;
        SpecularLighting(spec, specularColor, normalValue, -ViewDirection);
        color += spec;
    }

    if (detailMap.r != 0)
    {
        color = color * detailMap * DetailIntensity;
    }
    
    
    // Sample layers in texture array.
    float4 c0 = LayerMapArray.Sample(Sampler, float3(zValue, 0));
    c0.rgb = pow(c0.rgb, gamma);
    float4 c1 = LayerMapArray.Sample(Sampler, float3(zValue, 1));
    c1.rgb = pow(c1.rgb, gamma);
    float4 c2 = LayerMapArray.Sample(Sampler, float3(zValue, 2));
    c2.rgb = pow(c2.rgb, gamma);
    
    // Sample the blend map.
    float4 t = BlendMap.Sample(Sampler, input.Uv);
    
    float pRate = input.wPosition.y / MaxHeight;
    
    if (ActiveBlend.x == 1)
        color = BlendLerpHelper(color, c0, pRate, BlendPositionRate[0], 0.05f, 0.05f);
    if (ActiveBlend.y == 1)
        color = BlendLerpHelper(color, c1, pRate, BlendPositionRate[1], 0.15f, 0.15f);
    if (ActiveBlend.z == 1)
        color = BlendLerpHelper(color, c2, pRate, BlendPositionRate[2], 0.01f, 0.01f);
    
    float shadow = 1.0f;
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowPos, input.wPosition);
   
    ambient = float3(0, 0, 0);
    diffuse = float3(0, 0, 0);
    specular = float3(0, 0, 0);
    
    Material m2 = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l2 = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };
    
    ComputeDirectionalLight(m2, l2, SunColor, normalW, eye, A, D, S);
    
    color.rgb = pow(color.rgb, power);

    ambient += A * color;
    diffuse += D * color * shadow;
    specular += S * color * shadow;

    float4 result2 = float4(saturate(ambient + diffuse + specular), 1.0f);
        
    [flatten]
    if (fogEnabled == true)
    {
        float fogFactor = saturate((distanceToEye - FogStart) / FogRange);

        result2 = lerp(result2, SunColor, fogFactor);
    }

    

    output.tColor = result2 + float4(input.BrushColor, 1);
    output.tColor = output.tColor + float4(Line(input.wPosition), 0);
    

    output.pColor = float4(abs(input.Uv.x), abs(1 - input.Uv.y), 0, 1);

    return output;
}

void PS_Depth_Alpha(DomainOutput input)
{
    float4 diffuse = DiffuseMap.Sample(samLinear, input.Uv);

    // Don't write transparent pixels to the shadow map.
    clip(diffuse.a - 0.15f);
}

float4 PS_WireFrame(DomainOutput input) : SV_Target
{
    float d = distance(input.wPosition.xyz, ViewPosition);
    float s = saturate((d - MinDistance) / (MaxDistance - MinDistance));

    float4 colorFactor = 0;
    float4 magenta = float4(1, 0, 1, 1);
    float4 cyan = float4(0, 1, 1, 1);
    float4 yellow = float4(1, 1, 0, 1);

    if (s <= 0.5f)
        colorFactor = lerp(cyan, magenta, s / 0.5f);
    else
        colorFactor = lerp(magenta, yellow, (s - 0.5f) / 0.5f);
    
    float4 color = colorFactor;

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
        SetPixelShader(CompileShader(ps_5_0, PS_WireFrame()));
    }
}

technique11 T1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS_Depth()));
        SetDomainShader(CompileShader(ds_5_0, DS_Depth()));
        //SetPixelShader(CompileShader(ps_5_0, PS_Depth_Alpha()));
        SetPixelShader(NULL);

        SetRasterizerState(ShadowDepth);
    }

    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS_Depth()));
        SetDomainShader(CompileShader(ds_5_0, DS_Depth()));
        SetPixelShader(CompileShader(ps_5_0, PS_Depth_Alpha()));
    }
}