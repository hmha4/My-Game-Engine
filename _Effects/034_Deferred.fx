#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  Constant Buffers
//  --------------------------------------------------------------------------- //


//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //


//  --------------------------------------------------------------------------- //
//  Vertex Shader
//  --------------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : SV_POSITION;
};

VertexOutput VS(uint id : SV_VertexID)
{
    VertexOutput output;

    float2 uv = float2((id << 1) & 2, id & 2);

    output.Position = float4(uv, 0, 1);
    output.Position.x = output.Position.x * 2 - 1;
    output.Position.y = output.Position.y * -2 + 1;
    
    return output; 
}

//  --------------------------------------------------------------------------- //
//  Pixel Shader
//  --------------------------------------------------------------------------- //
Texture2D PositionGB;
Texture2D AlbedoGB;
Texture2D NormalGB;
Texture2D SpecularGB;
SamplerState Sampler;

float4 PS(VertexOutput input) : SV_TARGET
{
    int3 sampleIndices = int3(input.Position.xy, 0);
    float4 position = PositionGB.Load(sampleIndices);
    float4 albedo = AlbedoGB.Load(sampleIndices);
    float3 normal = NormalGB.Load(sampleIndices).xyz;
    float4 specularMap = SpecularGB.Load(sampleIndices);

    float4 shadowPos = mul(position, ShadowTransform);

    float gamma = 2.2f;
    float3 power = 1.0f / gamma;
    albedo.rgb = pow(albedo.rgb, gamma);

    float3 toEye = normalize(ViewPosition - position.xyz);
    //float distanceToEye = length(distanceToEye);
    //toEye = normalize(toEye);

    float4 ambient = 0;
    float4 diffuse = 0;
    float4 specular = 0;

    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, shadowPos, position.xyz);
   
    Specular.a = specularMap.a;
    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };

    float4 A, D, S;
    ComputeDirectionalLight(m, l, SunColor, normal, toEye, A, D, S);
    
    ambient += A * (shadow + 0.8f);
    diffuse += D * (shadow);
    specular += S * (shadow);

    [unroll]
    for (int i = 0; i < PointLightCount; i++)
    {
        ComputePointLight(m, PointLights[i], position.xyz, normal, toEye, A, D, S);

        ambient +=  A;
        diffuse +=  D;
        specular += S;
    }
    
    [unroll]
    for (i = 0; i < SpotLightCount; i++)
    {
        ComputeSpotLight(m, SpotLights[i], position.xyz, normal, toEye, A, D, S);

        ambient +=  A;
        diffuse +=  D;
        specular += S;
    }

    float3 light = float3(ambient.rgb + diffuse.rgb + specular.rgb);
    float4 color = float4(light * albedo.rgb, albedo.a);

    float4 specularColor = 1;
    
    SpecularLighting(specularColor, specularMap, normal, toEye);

    color.rgb += specularColor.rgb;
    

    float rimLight;
    float4 temp = color;
    
    rimLight = RimLighting(normal, toEye);
    temp *= rimLight;
    
    color += temp;
    
    float3 eye = ViewPosition - position.xyz;
    float distanceToEye = length(eye);

    float fogFactor = saturate((distanceToEye - 300) / 200);
    
    color = lerp(color, SunColor, fogFactor);

    color.rgb = pow(color.rgb, power);

    if (dot(albedo.rgb, 1.0f) == 0.0f)
        color.a = 0.0f;

    return color;
}

//  --------------------------------------------------------------------------- //
//  Technique & Pass
//  --------------------------------------------------------------------------- //
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

DepthStencilState DepthOff
{
    DepthEnable = false;
};

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DepthOff, 0);
    }
}