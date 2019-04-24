#include "000_Header.fx"

Texture2D ScreenDepth;
Texture2D FireShape;
Texture2D JitterTexture;

Texture2D<uint> PermTexture;

float Speed; //  Time * speed;

//  Fire params
float StepSize;
float NoiseScale;
float Roughness;
float FrequencyWeights[5];

bool Jitter;

//  Texture samples
SamplerState SamplerClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState SamplerRepeat
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer Gradients
{
    // 3D gradients
    
    const float3 Grad3[] =
    {
        float3(-1, -1, 0), // 0
        float3(-1, +1, 0), // 1
        float3(+1, -1, 0), // 2
        float3(+1, +1, 0), // 3

        float3(-1, 0, -1), // 4
        float3(-1, 0, +1), // 5
        float3(+1, 0, -1), // 6
        float3(+1, 0, +1), // 7

        float3(0, -1, -1), // 8
        float3(0, -1, +1), // 9
        float3(0, +1, -1), // 10
        float3(0, +1, +1), // 11

        // padding   
        float3(+1, +1, 0), // 12
        float3(-1, -1, 0), // 13
        float3(0, -1, +1), // 14
        float3(0, -1, -1) // 15
    };

    // 4D case is more regular

    const float4 Grad4[] =
    {
        // x, y, z
        float4(-1, -1, -1, 0), // 0
        float4(-1, -1, +1, 0), // 1
        float4(-1, +1, -1, 0), // 2
        float4(-1, +1, +1, 0), // 3
        float4(+1, -1, -1, 0), // 4
        float4(+1, -1, +1, 0), // 5
        float4(+1, +1, -1, 0), // 6
        float4(+1, +1, +1, 0), // 7
        // w, x, y
        float4(-1, -1, 0, -1), // 8
        float4(-1, +1, 0, -1), // 9
        float4(+1, -1, 0, -1), // 10
        float4(+1, +1, 0, -1), // 11
        float4(-1, -1, 0, +1), // 12
        float4(-1, +1, 0, +1), // 13
        float4(+1, -1, 0, +1), // 14
        float4(+1, +1, 0, +1), // 15
        // z, w, x
        float4(-1, 0, -1, -1), // 16
        float4(+1, 0, -1, -1), // 17
        float4(-1, 0, -1, +1), // 18
        float4(+1, 0, -1, +1), // 19
        float4(-1, 0, +1, -1), // 20
        float4(+1, 0, +1, -1), // 21
        float4(-1, 0, +1, +1), // 22
        float4(+1, 0, +1, +1), // 23
        // y, z, w
        float4(0, -1, -1, -1), // 24
        float4(0, -1, -1, +1), // 25
        float4(0, -1, +1, -1), // 26
        float4(0, -1, +1, +1), // 27
        float4(0, +1, -1, -1), // 28
        float4(0, +1, -1, +1), // 29
        float4(0, +1, +1, -1), // 30
        float4(0, +1, +1, +1) // 31
    };
};

// Defines
#define M_PI 3.14159265358979323846f
#define F3 0.333333333333
#define G3 0.166666666667

int Hash(float3 P)
{
    //P /= 256;   // normalize texture coordinate
    //return PermTexture.SampleLevel( SamplerRepeat, P.xy, 0 ).r ^ PermTexture.SampleLevel( SamplerRepeat, float2( P.z, 0 ), 0 ).r;
    return PermTexture.Load(int3(P.xy, 0)).r ^ PermTexture.Load(int3(P.z, 0, 0)).r;
}

void Simplex3D(const in float3 P, out float3 simplex[4])
{
    float3 T = P.xzy >= P.yxz;
    simplex[0] = 0;
    simplex[1] = T.xzy > T.yxz;
    simplex[2] = T.yxz <= T.xzy;
    simplex[3] = 1;
}

float Snoise3D(float3 P)
{
    // Skew the (x,y,z) space to determine which cell of 6 simplices we're in

    float s = dot(P, F3);
    float3 Pi = floor(P + s);
    float t = dot(Pi, G3);

    float3 P0 = Pi - t; // Unskew the cell origin back to (x,y,z) space
    float3 Pf0 = P - P0; // The x,y,z distances from the cell origin

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // To find out which of the six possible tetrahedra we're in, we need to
    // determine the magnitude ordering of x, y and z components of Pf0.

    float3 simplex[4];
    Simplex3D(Pf0, simplex);

    float n = 0;

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        float3 Pf = Pf0 - simplex[i] + G3 * i;
        int h = Hash(Pi + simplex[i]);
        float d = saturate(0.6f - dot(Pf, Pf));
        d *= d;
        n += d * d * dot(Grad3[h & 31], Pf);
    }

    return 32.0f * n;
}

// Turbulence function for 3D simplex noise
float Turbulence3D(float3 p)
{
    float res = 0;

    [loop] // forcing the loop results in better register allocation overall
    for (int i = 0; i < 5; i++, p *= 2)
        res += FrequencyWeights[i] * Snoise3D(p);

    return res;
}

struct VolumeVertex
{
    float4 Position : SV_Position;
    float3 oPosition : TEXCOORD0; // vertex position in model space
    float3 RayDir : TEXCOORD1; // ray direction in model space
};

VolumeVertex PerlinFireVS(VertexTextureNormalTangent input)
{
    VolumeVertex output;

    output.Position = mul(input.Position, World);
    output.RayDir = output.Position.xyz - ViewPosition;

    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.oPosition = input.Position.xyz; // supposed to have range -0.5 ... 0.5

    return output;
}

float4 PerlinFire3DPS(VolumeVertex input) : SV_Target
{
    float3 Dir = normalize(input.RayDir) * StepSize;
    float Offset = Jitter ? JitterTexture.Sample(SamplerRepeat, input.Position.xy / 256.0).r : 0;

    // Jitter initial position
    float3 Pos = input.oPosition + Dir * Offset;

    float3 resultColor = 0;
    float SceneZ = ScreenDepth.Load(int3(input.Position.xy, 0));
    
    while (true)
    {
        float4 ClipPos = mul(float4(Pos, 1), World);
        ClipPos = mul(ClipPos, View);
        ClipPos = mul(ClipPos, Projection);
        ClipPos.z /= ClipPos.w;

        // Break out of the loop if there's a blocking occluder or we're outside the fire volume
        if (ClipPos.z > SceneZ || any(abs(Pos) > 0.5))
            break;
    
        float3 NoiseCoord = Pos;
        NoiseCoord.y -= Time * Speed;

        // Evaluate turbulence function
        float Turbulence = abs(Turbulence3D(NoiseCoord * NoiseScale));

        float2 tc;
        tc.x = length(Pos.xz) * 2;
        tc.y = 0.5 - Pos.y - Roughness * Turbulence * pow((0.5 + Pos.y), 0.5);

        resultColor += StepSize * 12 * FireShape.SampleLevel(SamplerClamp, tc, 0);

        Pos += Dir;
    }
    

    return float4(resultColor, 1);
}

// Technique based on simplex 3D noise

DepthStencilState DisableDepthWrites
{
    DepthEnable = true;
    DepthWriteMask = 0;
};
BlendState EnableAdditiveBlending
{
    SrcBlend = INV_DEST_COLOR; // to avoid oversaturation
    DestBlend = One;
    BlendOp = Add;
    BlendEnable[0] = true;
};
RasterizerState MultisamplingEnabled
{
    MultisampleEnable = true;
};
technique10 PerlinFire3D
{
    pass P0
    {
        SetDepthStencilState(DisableDepthWrites, 0);
        SetBlendState(EnableAdditiveBlending, float4(0, 0, 0, 0), 0xffffffff);
        SetRasterizerState(MultisamplingEnabled);

        SetVertexShader(CompileShader(vs_5_0, PerlinFireVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PerlinFire3DPS()));
    }
}