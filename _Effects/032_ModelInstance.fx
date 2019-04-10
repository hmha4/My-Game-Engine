#include "000_Header.fx"

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------

Texture2D Transforms;
float4 ClipPlane[6];

struct VertexInput
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;

    matrix Transform : Instance0;
    uint InstID : SV_InstanceID;
};

bool AabbOutsideFrustumTest(float3 center, float3 extents)
{
    [unroll]
    for (int i = 0; i < 6; ++i)
    {
      // If the box is completely behind any of the frustum planes
      // then it is outside the frustum.
        [flatten]
        if (AabbBehindPlaneTest(center, extents, ClipPlane[i]))
        {
            return true;
        }
    }
    return false;
}

struct VertexOutput
{
    float4 Position : SV_POSITION0; // 0밖에 사용 불가함
    float3 wPosition : Position1;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;

    float4 ShadowPos : TEXCOORD1;
    float4 Clip : SV_CullDistance0; // VS에서 버텍스컬링하는 방법=>프러
};

VertexOutputDepth VS_Depth(VertexInput input)
{
    VertexOutputDepth output;

    float4 m0 = Transforms.Load(int3(BoneIndex * 4 + 0, input.InstID, 0));
    float4 m1 = Transforms.Load(int3(BoneIndex * 4 + 1, input.InstID, 0));
    float4 m2 = Transforms.Load(int3(BoneIndex * 4 + 2, input.InstID, 0));
    float4 m3 = Transforms.Load(int3(BoneIndex * 4 + 3, input.InstID, 0));

    matrix transform = matrix(m0, m1, m2, m3);
    World = mul(transform, input.Transform);

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, LightView);
    output.Position = mul(output.Position, LightProjection);

    output.Uv = input.Uv;
    
    return output;
}


VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    float4 m0 = Transforms.Load(int3(BoneIndex * 4 + 0, input.InstID, 0));
    float4 m1 = Transforms.Load(int3(BoneIndex * 4 + 1, input.InstID, 0));
    float4 m2 = Transforms.Load(int3(BoneIndex * 4 + 2, input.InstID, 0));
    float4 m3 = Transforms.Load(int3(BoneIndex * 4 + 3, input.InstID, 0));

    matrix transform = matrix(m0, m1, m2, m3);
    World = mul(transform, input.Transform);

    output.Position = mul(input.Position, World);
    output.wPosition = output.Position;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);

    output.Uv = input.Uv;
    output.ShadowPos = mul(float4(output.wPosition, 1.0f), ShadowTransform);

    [unroll]
    for (int i = 0; i < 6; i++)
    {
        if (AabbOutsideFrustumTest(output.wPosition, 0))
        {
            output.Clip = -1;
        }
        else
            output.Clip = 1;
    }

    return output;
}




//-----------------------------------------------------------------------------
// Pisxel Shader
//-----------------------------------------------------------------------------

SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 diffuseMap = DiffuseMap.Sample(Sampler, input.Uv);
    
    //  아티스트들이 텍스쳐를 만들때 모니터를 보고 만들기 때문에
    //  감마색 공간에서 색을 정한다.
    //  그래서 내가 감마색 공간을 사용하고 싶으면 2번 Gamma Correct를 하지 않기위해서
    //  아무것도 하기 전에 이미지 색의 감마색 공간을 제거한다.
    float gamma = 2.2f;
    float3 power = 1.0f / gamma;
    diffuseMap.rgb = pow(diffuseMap.rgb, gamma);

    float4 specularMap = SpecularMap.Sample(Sampler, input.Uv);
    float3 normalMap = NormalMap.Sample(Sampler, input.Uv);

    float diffuseFactor = saturate(dot(diffuseMap, 1));
    float specularFactor = saturate(dot(specularMap, 1));
    float normalFactor = saturate(dot(normalMap, 1));
    
    float3 normal = input.Normal;

    if (normalFactor > 0.0f)
        normal = NormalSampleToWorldSpace(normalMap, input.Normal, input.Tangent);

    float3 toEye = ViewPosition - input.wPosition;
    float dEye = length(toEye);
    toEye /= dEye;  //  normalize

    float4 ambient = 0;
    float4 diffuse = 0;
    float4 specular = 0;

    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowPos, input.wPosition);

    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;
    ComputeDirectionalLight(m, l, SunColor, normal, toEye, A, D, S);

    diffuseMap.rgb = pow(diffuseMap.rgb, power);

    if (diffuseFactor > 0.0f)
    {
        A *= diffuseMap;
        D *= diffuseMap;
        S *= diffuseMap;
    }
    
    ambient += A;
    diffuse += shadow * D;

    if (specularFactor > 0.0f)
        specular += shadow * S;
   
    [unroll]
    for (int i = 0; i < PointLightCount; i++)
    {
        ComputePointLight(m, PointLights[i], input.wPosition, normal, toEye, A, D, S);

        ambient += A;
        diffuse += D;
        specular += S;
    }
    
    [unroll]
    for (i = 0; i < SpotLightCount; i++)
    {
        ComputeSpotLight(m, SpotLights[i], input.wPosition, normal, toEye, A, D, S);

        ambient += A;
        diffuse += D;
        specular += S;
    }
    
    float4 color = float4(ambient + diffuse + specular);
    

    float4 specularColor = 1;
    
    if (specularFactor > 0.0f)
        SpecularLighting(specularColor, specularMap, normal, toEye);
    if (specularFactor == 0.0f)
        SpecularLighting(specularColor, normal, toEye);
    
    color.rgb += (specularColor.rgb * diffuseMap.rgb);

    float fogFactor = saturate((dEye - 300) / 200);

    color.rgb = lerp(color.rgb, SunColor.rgb, fogFactor);

    //color.rgb = pow(color.rgb, power);

    //float rimLight;
    //float4 temp = color;
    
    //rimLight = RimLighting(normal, toEye);
    //temp *= rimLight;
    
    //color += temp;
    
   
    clip(color.a - 0.15f);

    return color;
}

void PS_Depth_Alpha(VertexOutputDepth input)
{
    float4 diffuse = DiffuseMap.Sample(samLinear, input.Uv);

    // Don't write transparent pixels to the shadow map.
    clip(diffuse.a - 0.15f);
}

PixelOutPut PS_GB(VertexOutput input)
{
    PixelOutPut output;

    float4 diffuseMap = DiffuseMap.Sample(Sampler, input.Uv);
    float3 normalMap = NormalMap.Sample(Sampler, input.Uv);
    float4 specularMap = SpecularMap.Sample(Sampler, input.Uv);
    float normalFactor = saturate(dot(normalMap, 1));
    
    float3 normal = input.Normal;

    if (normalFactor > 0.0f)
        normal = NormalSampleToWorldSpace(normalMap, input.Normal, input.Tangent);
    
    specularMap.rgb *= Specular.rgb;
    specularMap.a = Specular.a;
    diffuseMap.rgb *= Diffuse.rgb;
    diffuseMap.r += 0.00001f;
    diffuseMap.g += 0.00001f;
    diffuseMap.b += 0.00001f;

    clip(diffuseMap.a - 0.15f);

    output.Position = float4(input.wPosition, 1.0f);
    output.Normal = float4(normal, 1.0f);
    output.Diffuse = diffuseMap;
    output.Specular = specularMap;

    return output;
}

//-----------------------------------------------------------------------------
// Techinques
//-----------------------------------------------------------------------------
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

RasterizerState Cull
{
    CullMode = None;
};

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        //SetGeometryShader(CompileShader(gs_5_0, GS()));

        //SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(Cull);
    }
}

technique11 T1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Depth()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Depth_Alpha()));

        SetRasterizerState(ShadowDepth);
    }
}

technique11 T2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_GB()));

        SetRasterizerState(Cull);
    }
}