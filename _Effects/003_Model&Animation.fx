#include "000_Header.fx"


//=====================================================//
// VertexShader
//=====================================================//
struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float3 wPosition : Position1;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;
};

VertexOutput VS_Bone(VertexTextureNormalTangentBlend input)
{
    VertexOutput output;

    World = BoneWorld();
    output.Position = mul(input.Position, World);
    output.wPosition = output.Position.xyz;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);

    output.Uv = input.Uv;
    

    return output; // 반환값이 픽셀의 위치
}

VertexOutput VS_Animation(VertexTextureNormalTangentBlend input)
{
    VertexOutput output;

    World = SkinWorld(input.BlendIndices, input.BlendWeights);
    output.Position = mul(input.Position, World);
    output.wPosition = output.Position.xyz;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);

    output.Uv = input.Uv;
    

    return output; // 반환값이 픽셀의 위치
}

//=====================================================//
// PixelShader
//=====================================================//
//SamplerState Sampler; // 기본값사용시

SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    float3 normal = input.Normal;

    float3 normalMap = NormalMap.Sample(Sampler, input.Uv);
    if (normalMap.b > 0.0f)
        normal = NormalSampleToWorldSpace(normalMap, input.Normal, input.Tangent);

    float3 toEye = normalize(ViewPosition - input.wPosition);

    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);

    //float shadow = float3(1.0f, 1.0f, 1.0f);
    //shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowPos);

    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;
    float4 sunColor = float4(1, 1, 1, 1);
    ComputeDirectionalLight(m, l, sunColor, normal, toEye, A, D, S);
    ambient += A;
    diffuse += /*shadow * */D;
    specular += /*shadow * */S;
    
    float4 diffuseMap = DiffuseMap.Sample(Sampler, input.Uv);
    if (diffuseMap.r > 0.0f)
    {
        ambient *= diffuseMap;
        diffuse *= diffuseMap;
        specular *= diffuseMap;
    }

    float3 specularMap = SpecularMap.Sample(Sampler, input.Uv);
    if (specularMap.r > 0.0f)
        specular *= specularMap;
       

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
    
    float4 color = float4(ambient + diffuse + specular, 1);
    
   

    return color;
}


//=====================================================//
// technique
//=====================================================//
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Bone()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Animation()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}