#include "000_Header.fx"

float3 HUEtoRGB(in float H)
{
    float R = abs(H * 6 - 3) - 1;
    float G = 2 - abs(H * 6 - 2);
    float B = 2 - abs(H * 6 - 4);
    return saturate(float3(R, G, B));
}

float3 HSVtoRGB(in float3 HSV)
{
    float3 RGB = HUEtoRGB(HSV.x);
    return ((RGB - 1) * HSV.y + 1) * HSV.z;
}


float4 WorldFrustumPlanes[6];
matrix ReflectionView;
float FogStart;
float FogRange;

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct VertexInput
{
    float4 Position : POSITION0;
    
    float3 TransPosition : INSTANCE0;
    matrix DeTransWorld : INSTANCE1;
    matrix RotateWorld : INSTANCE5;
    float2 Size : INSTANCE9;
    uint TextureNumber : INSTANCE10;

    uint InstanceId : SV_INSTANCEID0;
};

struct VertexOutput
{
    float4 Center : POSITION0;
    float2 Size : SIZE0;
    uint TextureNumber : GRASS0;
    matrix RotateWorld : MATRIX0;
    matrix DeTransWorld : MATRIX4;
    uint InstanceId : INSTANCEID0;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    output.Center = float4(input.TransPosition, 1);

    output.Size = input.Size;
    output.TextureNumber = input.TextureNumber;
    output.RotateWorld = input.RotateWorld;
    output.DeTransWorld = input.DeTransWorld;
    output.InstanceId = input.InstanceId;
    
    return output;
}

// --------------------------------------------------------------------- //
//  Geometry Shader
// --------------------------------------------------------------------- //
struct GeometryOutput
{
    float4 Position : SV_POSITION;
    float3 wPosition : Position1;
    float2 Uv : UV0;
    uint TextureNumber : GRASS0;
    float4 ShadowTransform : UV1;
    uint InstanceId : INSTANCEID0;
};

static const float2 TexCoord[4] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
};

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

[maxvertexcount(4)] //  최대 넘겨 보낼 버텍스 개수
void GS(point VertexOutput input[1], uint PrimitiveID : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    float4 root = input[0].Center;

    float cameraDistance = length(ViewPosition.xz - root.xz);

    // Properties of the grass blade
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;
    
	// Animation
    float toTheLeft = sin(Time);


    // Generating vertices
    float3 up = float3(0, 1, 0);
    float3 look = ViewPosition - input[0].Center.xyz;
    look.y = 0.0f;
    look = normalize(look);

    float3 right = cross(up, look);

    
    float3 boxExtents = (halfWidth + halfHeight) * 0.5f;
    if (AabbOutsideFrustumTest(input[0].Center.xyz, boxExtents))
    {
        return;
    }
    //  4 정점 계산
    float4 v[4];

    v[0] = float4(input[0].Center.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Center.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Center.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Center.xyz - halfWidth * right + halfHeight * up, 1.0f);

    GeometryOutput output;

    [unroll]
    for (int i = 0; i < 4; ++i) //  ++i 가 i++ 보다 빠름
    {
        output.wPosition = input[0].Center.xyz;

        output.Position = mul(v[i], input[0].RotateWorld);
        output.Position = mul(output.Position, input[0].DeTransWorld);
        output.Position = mul(output.Position, View);
        output.Position = mul(output.Position, Projection);

        output.ShadowTransform = mul(float4(output.wPosition, 1), ShadowTransform);
  
        output.Uv = TexCoord[i];
        output.TextureNumber = input[0].TextureNumber;
        output.InstanceId = input[0].InstanceId;

        stream.Append(output);
    }

}

[maxvertexcount(4)] //  최대 넘겨 보낼 버텍스 개수
void GS_Depth(point VertexOutput input[1], uint PrimitiveID : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    float4 root = input[0].Center;

    float cameraDistance = length(ViewPosition.xz - root.xz);

    // Properties of the grass blade
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;
    
	// Animation
    float toTheLeft = sin(Time);


    // Generating vertices
    float3 up = float3(0, 1, 0);
    float3 lightPos = -LightDirection * 500.0f;
    float3 look = lightPos - input[0].Center.xyz;
    look.y = 0.0f;
    look = normalize(look);

    float3 right = cross(up, look);

    //  4 정점 계산
    float4 v[4];

    v[0] = float4(input[0].Center.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Center.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Center.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Center.xyz - halfWidth * right + halfHeight * up, 1.0f);


    GeometryOutput output;
    [unroll]
    for (int i = 0; i < 4; ++i) //  ++i 가 i++ 보다 빠름
    {
        //output.Position = mul(v[i], LightView);
        //output.Position = mul(output.Position, LightProjection);
     
        //output.Uv = TexCoord[i];
        //output.BillboardNum = input[0].BillboardNum;

        //stream.Append(output);

        output.wPosition = v[i].xyz;
        output.Position = mul(v[i], LightView);
        output.Position = mul(output.Position, LightProjection);

        output.ShadowTransform = mul(v[i], ShadowTransform);
        
        output.Uv = TexCoord[i];
        output.TextureNumber = input[0].TextureNumber;
        output.InstanceId = input[0].InstanceId;

        stream.Append(output);
    }

}

[maxvertexcount(4)] //  최대 넘겨 보낼 버텍스 개수
void GS_Reflection(point VertexOutput input[1], uint PrimitiveID : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    float4 root = input[0].Center;

    float cameraDistance = length(ViewPosition.xz - root.xz);

    // Properties of the grass blade
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;
    
	// Animation
    float toTheLeft = sin(Time);


    // Generating vertices
    float3 up = float3(0, 1, 0);
    float3 lightPos = -LightDirection * 500.0f;
    float3 look = lightPos - input[0].Center.xyz;
    look.y = 0.0f;
    look = normalize(look);

    float3 right = cross(up, look);

    //  4 정점 계산
    float4 v[4];

    v[0] = float4(input[0].Center.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Center.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Center.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Center.xyz - halfWidth * right + halfHeight * up, 1.0f);


    GeometryOutput output;
    [unroll]
    for (int i = 0; i < 4; ++i) //  ++i 가 i++ 보다 빠름
    {
        //output.Position = mul(v[i], LightView);
        //output.Position = mul(output.Position, LightProjection);
     
        //output.Uv = TexCoord[i];
        //output.BillboardNum = input[0].BillboardNum;

        //stream.Append(output);

        output.wPosition = v[i].xyz;
        output.Position = mul(v[i], ReflectionView);
        output.Position = mul(output.Position, Projection);

        output.ShadowTransform = mul(v[i], ShadowTransform);
        
        output.Uv = TexCoord[i];
        output.TextureNumber = input[0].TextureNumber;
        output.InstanceId = input[0].InstanceId;

        stream.Append(output);
    }

}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2DArray Map;
sampler Sampler
{
    AddressU = WRAP;
    AddressV = WRAP;
    Filter = COMPARISON_ANISOTROPIC;
    MaxAnisotropy = 16;
    MaxLOD = 2.0f;
};

float4 PS(GeometryOutput input) : SV_TARGET
{
    //번호 0번 0번그림 = > TextureNumber
    //번호 1번 1번그림
    
    float4 flower = Map.Sample(Sampler, float3(input.Uv, input.TextureNumber % 7));

    float3 toEye = ViewPosition - input.wPosition;
    float distanceToEye = length(toEye);
    
    toEye = normalize(toEye);

    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);

    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowTransform);

    shadow = min(0.5f, shadow);

    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };


    float4 A, D, S;
    float3 normal = float3(0, 1, 0);
    ComputeDirectionalLight(m, l, SunColor, normal, toEye, A, D, S);
    ambient += A;
    diffuse += D * shadow;
    specular += S * shadow;
    
    float4 color = float4(ambient + diffuse + specular, 1.0f);
    color *= flower;

    clip(color.a - 0.5f);
    float fogFactor = saturate((distanceToEye - FogStart) / FogRange);

    color = lerp(color, SunColor, fogFactor);

    return color;
}

void PS_Depth(GeometryOutput input)
{
    float4 color = Map.Sample(Sampler, float3(input.Uv, input.TextureNumber % 7));
    
    clip(color.a - 0.5f);
}

PixelOutPut PS_GB(GeometryOutput input)
{
    PixelOutPut output;

    float4 diffuseMap = Map.Sample(Sampler, float3(input.Uv, input.TextureNumber % 7));
    float3 normalMap = NormalMap.Sample(Sampler, input.Uv);
    float4 specularMap = SpecularMap.Sample(Sampler, input.Uv);
    float normalFactor = saturate(dot(normalMap, 1));
    
    specularMap.rgb *= Specular.rgb;
    specularMap.a = Specular.a;
    diffuseMap.rgb *= Diffuse.rgb;

    clip(diffuseMap.a - 0.5f);

    output.Position = float4(input.wPosition, 1.0f);
    output.Normal = float4(normalMap, 1.0f);
    output.Diffuse = diffuseMap;
    output.Specular = specularMap;

    return output;
}


// --------------------------------------------------------------------- //
//  Techniques
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

technique11 T1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Depth()));
        SetPixelShader(CompileShader(ps_5_0, PS_Depth()));
    }
    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Depth()));
        //SetPixelShader(CompileShader(ps_5_0, PS_Depth_Alpha()));
        SetPixelShader(NULL);

        SetRasterizerState(ShadowDepth);
    }
}

technique11 T2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS_GB()));
    }
}

technique11 T3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Reflection()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}