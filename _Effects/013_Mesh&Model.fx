#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float3 wPosition : POSITION1;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

VertexOutput VS(VertexTextureNormalTangent input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.wPosition = output.Position;

    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Tangent = WorldTangent(input.Tangent);
    output.Normal = WorldNormal(input.Normal);
    output.Uv = input.Uv;

    return output;
}

VertexOutput VS_Model(VertexTextureNormalTangent input)
{
    VertexOutput output;

    World = Bones[BoneIndex];
    output.Position = mul(input.Position, World);
    output.wPosition = output.Position;

    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Tangent = WorldTangent(input.Tangent);
    output.Normal = WorldNormal(input.Normal);
    output.Uv = input.Uv;

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
SamplerState Sampler
{
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    float3 normalMap = NormalMap.Sample(Sampler, input.Uv);
    float3 normal = NormalSampleToWorldSpace(normalMap, input.Normal, input.Tangent);
    
    float3 toEye = normalize(ViewPosition - input.wPosition);

    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);


    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;
    ComputeDirectionalLight(m, l, normal, toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;

    ambient *= DiffuseMap.Sample(Sampler, input.Uv);
    diffuse *= DiffuseMap.Sample(Sampler, input.Uv);
    specular *= DiffuseMap.Sample(Sampler, input.Uv);

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
    color.a = Diffuse.a;

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
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Model()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}