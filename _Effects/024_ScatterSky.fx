#include "000_Header.fx"

//  상수는 static const로 선언
static const float PI = 3.14159265f;
static const float InnerRadius = 6356.7523142; //  지구 반지름
static const float OuterRadius = 6356.7523142 * 1.0157313; //  대기 반지름



float ESun = 20.0f; //  태양의 밝기
float Kr = 0.0025f; //  Rayleigh
float Km = 0.0010f; //  Mie

float KrESun = 0.0025f * 20.0f;
float KmESun = 0.0010f * 20.0f;
float Kr4PI = 0.0025f * 4.0f * PI;
float Km4PI = 0.0010f * 4.0f * PI;

float Scale = 1.0f / (OuterRadius - InnerRadius); //   비율 
float2 RayleighMieScaleHeight = { 0.25, 0.1 };

float g = -0.990f;
float g2 = -0.990f * -0.990f;
float Exposure = -2.0f; //  밝은 빛을 강조 시키기 위한 상수

float3 WaveLength;
int SampleCount;
float3 InvWaveLength;
float3 WaveLengthMie;
//float4 SunColor;

float MoonAlpha = 0.0f;

float NumTiles = 16.25f;
float NoiseTime = 0.0f;

SamplerState TrilinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

RasterizerState CullModeOff
{
    CullMode = None;
};

DepthStencilState DepthOff
{
    DepthEnable = false;
};

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

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
    float3 oPosition : POSITION1;
};

struct PixelCloudInput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
    float2 oUv : UV1;
};

PixelInput VS_Scattering(VertexTexture input)
{
    PixelInput output;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.oPosition = -input.Position.xyz;
    output.Uv = input.Uv;

    return output;
}

Texture2D RayleighMap;
SamplerState RayleighSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

Texture2D MieMap;
SamplerState MieSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

Texture2D StarfieldMap;
SamplerState StarfieldSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

// Phase 위상, 단계
float GetRayleighPhase(float c)
{
    return 0.75f * (1.0f + c);
}

float GetMiePhase(float c1, float c2)
{
    float3 result = 0;

    result.x = 1.5f * ((1.0f - g2) / (2.0f + g2));
    result.y = 1.0f + g2;
    result.z = 2.0f * g;
    return result.x * (1.0 + c2) / pow(result.y - result.z * c1, 1.5);
}

float3 HDR(float3 LDR)
{
    return 1.0f - exp(Exposure * LDR);
}

float4 PS_Scattering(PixelInput input) : SV_TARGET
{
    float3 sunDirection = -normalize(LightDirection);

    float temp = dot(sunDirection, input.oPosition) / length(input.oPosition);
    float temp2 = temp * temp;

    float3 rSamples = RayleighMap.Sample(RayleighSampler, input.Uv);
    float3 mSamples = MieMap.Sample(MieSampler, input.Uv);

    float3 color = 0;

    color = GetRayleighPhase(temp2) * rSamples + GetMiePhase(temp, temp2) * mSamples;
    //color = GetMiePhase(temp, temp2) * mSamples;
    //color = GetRayleighPhase(temp2) * rSamples;
    color = HDR(color);

    color += max(0, (1 - color)) * float3(0.05f, 0.05f, 0.1f);
    
    float4 starField = saturate(StarfieldMap.Sample(StarfieldSampler, input.Uv) * LightDirection.y);
    return float4(color, 1) + saturate(starField);
}

///////////////////////////////////////////////////////////////////////////////

struct PixelTargetInput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
};


PixelTargetInput VS_Target(VertexTexture input)
{
    PixelTargetInput output;
    
    //  NDC 공간
    output.Position = input.Position;
    output.Uv = input.Uv;
    
    return output;
}

struct PixelTargetOutput
{
    float4 RColor : SV_TARGET0;
    float4 MColor : SV_TARGET1;
};

float HitOuterSphere(float3 position, float3 direction)
{
    float3 light = -position;
    
    float b = dot(light, direction);
    float c = dot(light, light);

    float d = c - b * b;
    float q = sqrt(OuterRadius * OuterRadius - d);

    float t = b;
    t += q;

    return t;
}

float2 GetDensityRatio(float height)
{
    float altitude = (height - InnerRadius) * Scale;

    return exp(-altitude / RayleighMieScaleHeight.xy);
}

float2 GetDistance(float3 p1, float3 p2)
{
    float2 opticalDepth = 0;

    float3 temp = p2 - p1;
    float far = length(temp);
    float3 direction = temp / far;

    float sampleLength = far / SampleCount;
    float scaledLength = sampleLength * Scale;

    float3 sampleRay = direction * sampleLength;
    p1 += sampleRay * 0.5f;

    for (int i = 0; i < SampleCount; i++)
    {
        float height = length(p1);
        opticalDepth += GetDensityRatio(height);

        p1 += sampleRay;
    }
    
    return opticalDepth * scaledLength;
}

PixelTargetOutput PS_Target(PixelTargetInput input) : SV_TARGET
{
    PixelTargetOutput output;
    
    float2 uv = input.Uv;

    float3 pointPv = float3(0, InnerRadius + 1e-3f, 0.0f);
    float angleY = 100.0f * uv.x * PI / 180.0f;
    float angleXZ = PI * uv.y;

    float3 direction;
    direction.x = sin(angleY) * cos(angleXZ);
    direction.y = cos(angleY);
    direction.z = sin(angleY) * sin(angleXZ);
    direction = normalize(direction);

    float farPvPa = HitOuterSphere(pointPv, direction);
    float3 ray = direction;

    float3 pointP = pointPv;
    float sampleLength = farPvPa / SampleCount;
    float scaledLength = sampleLength * Scale;
    float3 sampleRay = ray * sampleLength;
    pointP += sampleRay * 0.5f;

    float3 sunDir = normalize(LightDirection);
    sunDir.y *= -1.0f;

    float3 rayleighSum = 0;
    float3 mieSum = 0;

    for (int i = 0; i < SampleCount; i++)
    {
        float pHeight = length(pointP);
        
        float2 densityRatio = GetDensityRatio(pHeight);
        densityRatio *= scaledLength;

        float2 viewerOpticalDepth = GetDistance(pointP, pointPv);

        float farPPc = HitOuterSphere(pointP, sunDir);
        float2 sunOpticalDepth = GetDistance(pointP, pointP + sunDir * farPPc);

        float2 opticalDepthP = sunOpticalDepth.xy + viewerOpticalDepth.xy;
        float3 attenuation = exp(-Kr4PI * InvWaveLength * opticalDepthP.x - Km4PI * opticalDepthP.y);

        rayleighSum += densityRatio.x * attenuation;
        mieSum += densityRatio.y * attenuation;

        pointP += sampleRay;
    }

    float3 rayleigh = rayleighSum * KrESun;
    float3 mie = mieSum * KmESun;

    rayleigh *= InvWaveLength;
    mie *= WaveLengthMie;

    output.RColor = float4(rayleigh, 1);
    output.MColor = float4(mie, 1);

    return output;
}

///////////////////////////////////////////////////////////////////////////////

Texture2D MoonMap;
Texture2D MoonGlowMap;

PixelInput VS_Moon(VertexTexture input)
{
    PixelInput output;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.oPosition = -input.Position.xyz;
    output.Uv = input.Uv;

    return output;
}

float4 PS_Moon(PixelInput input) : SV_TARGET
{
    float4 color = 0;

    color = MoonMap.Sample(TrilinearSampler, input.Uv);
    color.a *= (MoonAlpha * 1.5f);

    return color;
}

float4 PS_MoonGlow(PixelInput input) : SV_TARGET
{
    float4 color = 0;

    color = MoonGlowMap.Sample(TrilinearSampler, input.Uv);
    color.a *= MoonAlpha;

    return color;
}

///////////////////////////////////////////////////////////////////////////////
//cbuffer PS_CloudBuffer : register(b10)
//{
//    float NoiseTime;
//}

static const float CloudCover = -0.1;
static const float CloudSharpness = 0.125;


Texture2D CloudMap;
SamplerState CloudSampler;


PixelCloudInput VS_Cloud(VertexTexture input)
{
    PixelCloudInput output;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    output.Uv = (input.Uv * NumTiles);
    output.oUv = input.Uv;

    return output;
}



static const float ONE = 0.00390625;
static const float ONEHALF = 0.001953125;
// The numbers above are 1/256 and 0.5/256, change accordingly
// if you change the code to use another texture size.

float fade(float t)
{
  // return t*t*(3.0-2.0*t);
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}


float noise(float2 P)
{
    float2 Pi = ONE * floor(P) + ONEHALF;
    float2 Pf = frac(P);

    float2 grad00 = CloudMap.Sample(CloudSampler, Pi).rg * 4.0 - 1.0;
    float n00 = dot(grad00, Pf);

    float2 grad10 = CloudMap.Sample(CloudSampler, Pi + float2(ONE, 0.0)).rg * 4.0 - 1.0;
    float n10 = dot(grad10, Pf - float2(1.0, 0.0));

    float2 grad01 = CloudMap.Sample(CloudSampler, Pi + float2(0.0, ONE)).rg * 4.0 - 1.0;
    float n01 = dot(grad01, Pf - float2(0.0, 1.0));

    float2 grad11 = CloudMap.Sample(CloudSampler, Pi + float2(ONE, ONE)).rg * 4.0 - 1.0;
    float n11 = dot(grad11, Pf - float2(1.0, 1.0));

    float2 n_x = lerp(float2(n00, n01), float2(n10, n11), fade(Pf.x));

    float n_xy = lerp(n_x.x, n_x.y, fade(Pf.y));

    return n_xy;
}


float4 PS_Cloud(PixelCloudInput input) : SV_Target
{
    float n = noise(input.Uv + NoiseTime);
    float n2 = noise(input.Uv * 2 + NoiseTime);
    float n3 = noise(input.Uv * 4 + NoiseTime);
    float n4 = noise(input.Uv * 8 + NoiseTime);
	
    float nFinal = n + (n2 / 2) + (n3 / 4) + (n4 / 8);
	
    float c = CloudCover - nFinal;
    if (c < 0) 
        c = 0;
 
    float CloudDensity = 1.0 - pow(CloudSharpness, c);
    
    float4 retColor = CloudDensity;
    retColor *= SunColor;
    
    float uvX = abs(0.5f - input.oUv.x) + 0.5f;
    float uvY = abs(0.5f - input.oUv.y) + 0.5f;
    
    retColor.a = uvX > 0.8 ? lerp(0.0f, retColor.a * 0.85f, (1 - uvX) / 0.2f) : retColor.a;
    retColor.a = uvY > 0.8 ? lerp(0.0f, retColor.a * 0.85f, (1 - uvY) / 0.2f) : retColor.a;
    
    return retColor;
}

technique11 T0
{
    pass P0
    {
        SetDepthStencilState(DepthOff, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Scattering()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Scattering()));
    }

    pass P1
    {
        SetDepthStencilState(DepthOff, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Target()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Target()));
    }

    // Moon
    pass P2
    {
        SetDepthStencilState(DepthOff, 0);
        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS_Moon()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Moon()));
    }

    // Moon Glow
    pass P3
    {
        SetDepthStencilState(DepthOff, 0);
        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS_Moon()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MoonGlow()));
    }

    pass P4
    {
        SetRasterizerState(CullModeOff);
        SetDepthStencilState(DepthOff, 0);
        SetBlendState(BlendOn, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS_Cloud()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Cloud()));
    }
};
