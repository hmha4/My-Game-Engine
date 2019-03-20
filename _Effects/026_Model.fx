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
};

VertexOutput VS(VertexTextureNormalTangent input)
{
    VertexOutput output;

    //World = BoneWorld();
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

    float3 toEye = normalize(ViewPosition - input.wPosition);

    float4 ambient = 0;
    float4 diffuse = 0;
    float4 specular = 0;

    //float shadow = float3(1.0f, 1.0f, 1.0f);
    //shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowPos);

    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;
    float4 sunColor = float4(1, 1, 1, 1);
    ComputeDirectionalLight(m, l, sunColor, normal, toEye, A, D, S);

    if (diffuseFactor > 0.0f)
    {
        A *= diffuseMap;
        D *= diffuseMap;
        S *= diffuseMap;
    }
    
    ambient += A;
    diffuse += /*shadow * */D;

    if (specularFactor > 0.0f)
        specular += /*shadow * */S;
   
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
    
    if (specularFactor > 0.0f)
        SpecularLighting(color, specularMap, normal, toEye);
    if (specularFactor == 0.0f)
        SpecularLighting(color, normal, toEye);
    
    float rimLight;
    float4 temp = color;
    
    rimLight = RimLighting(normal, toEye);
    temp *= rimLight;
    
    color += temp;
    color.rgb = pow(color.rgb, power);
    
    return color;
}


//=====================================================//
// technique
//=====================================================//
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

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}