#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //
float4 FrustumPlanes[6];

float MinWidth;
float MinHeight;

float2 TimeVector;
float2 WindVector;
float2 WindAccel;

// Our constants
float3 LightPosition = float3(0, 200, 10);

static const float kHalfPi = 1.5707;
static const float kQuarterPi = 0.7853;
static const float kOscillateDelta = 0.25;
static const float kWindCoeff = 50.0f;

// --------------------------------------------------------------------- //
//  States
// --------------------------------------------------------------------- //
RasterizerState Rasterizer
{
    FillMode = Solid;
    CullMode = None;
};

BlendState Blend
{
    AlphaToCoverageEnable = true;

    BlendEnable[0] = true;
    RenderTargetWriteMask[0] = 1 | 2 | 4 | 8; // Write all colors R | G | B | A

    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
};

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //



bool AabbOutsideFrustumTest(float3 center, float3 extents)
{
    [unroll]
    for (int i = 0; i < 6; ++i)
    {
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum.
        [flatten]
        if (AabbBehindPlaneTest(center, extents, FrustumPlanes[i]))
        {
            return true;
        }
    }
	
    return false;
}

struct VertexInput
{
    float4 Position : POSITION0;
    float4 iPosition : INSTANCE0;
    float2 Uv : INSTANCE1;
    float3 Normal : INSTANCE2;
};

struct GeometryInput
{
    float4 Position : POSITION0;
    float3 Normal : NORMAL0;
};

GeometryInput VS_Grass(VertexTextureNormal input)
{
    GeometryInput output;

    output.Position = input.Position;
    output.Normal = input.Normal;
    
    return output;
}

GeometryInput VS_Flower(VertexTextureNormal input)
{
    GeometryInput output;

    // Generate a random number between 0.0 to 1.0 by using the root position (which is randomized by the CPU)
    float random = sin(kHalfPi * frac(input.Position.x) + kHalfPi * frac(input.Position.z));

    float cameraDistance = length(ViewPosition.xz - input.Position.xz);

	// Properties of the grass blade
    float minHeight = 10.5f;
    float sizeY = minHeight + (random / 5);

    output.Position = input.Position + float4(0, sizeY * 0.5f, 0, 0);
    output.Normal = input.Normal;

    return output;
}

// --------------------------------------------------------------------- //
//  Geometry Shader
// --------------------------------------------------------------------- //
struct GeometryOutput
{
    float4 Position : SV_POSITION;
    float4 wPosition : Position1;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 VertexToLight : NORMAL1;
    float3 VertexToCamera : NORMAL2;
    float Random : NORMAL4;
    float4 ShadowTransform : UV1;
    //uint InstancedId : InstanceID0;
};

GeometryOutput createGEO_OUT_Grass()
{
    GeometryOutput output;
	
    output.Position = float4(0, 0, 0, 0);
    output.wPosition = float4(0, 0, 0, 0);
    output.Normal = float3(0, 0, 0);
    output.Uv = float2(0, 0);
    output.VertexToLight = float3(0, 0, 0);
    output.VertexToCamera = float3(0, 0, 0);
    output.Random = 0;
    output.ShadowTransform = float4(0, 0, 0, 0);
    //output.InstancedId = 0;

    return output;
}

static const float2 TexCoord[4] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
};

static const float PI = 3.14159265358979323846264338327950288f;

[maxvertexcount(12)]
void GS_Grass(point GeometryInput points[1], uint PrimitiveID : SV_PrimitiveID, inout TriangleStream<GeometryOutput> stream)
{
    float4 root = points[0].Position;

    float minY = root.y - 1.0f;
    float maxY = root.y + 1.0f;

    float3 minV = float3(root.x - 1.0f, minY, root.z - 1.0f);
    float3 maxV = float3(root.x + 1.0f, maxY, root.z + 1.0f);

    float3 boxCenter = (minV + maxV) * 0.5f;
    float3 boxExtents = (maxV - minV) * 0.5f;

    if (AabbOutsideFrustumTest(boxCenter, boxExtents))
        return;

    LightPosition = -LightDirection * 200;
    
	// Generate a random number between 0.0 to 1.0 by using the root position (which is randomized by the CPU)
    float random = sin(kHalfPi * frac(root.x) + kHalfPi * frac(root.z));
    float randomRotation = random;
    
	// Rotate in Z-axis
    float3x3 rotationMatrix =
    {
        cos(randomRotation), 0, sin(randomRotation),
		0, 1, 0,
		-sin(randomRotation), 0, cos(randomRotation)
    };

    const float rotFactor = PI * 0.33f;

    float3x3 rot[3] =
    {
        {
            cos(rotFactor), 0, sin(rotFactor),
		    0, 1, 0,
		    -sin(rotFactor), 0, cos(rotFactor)
        },
        {
            cos(PI - rotFactor), 0, sin(PI - rotFactor),
		    0, 1, 0,
		    -sin(PI - rotFactor), 0, cos(PI - rotFactor)
        },
        {
            cos(PI), 0, sin(PI),
		    0, 1, 0,
		    -sin(PI), 0, cos(PI)
        }
};

	/////////////////////////////////
	// Generating vertices
	/////////////////////////////////

    const uint vertexCount = 4;
    const uint billboardCount = 3;
    GeometryOutput v[vertexCount * billboardCount] =
    {
        createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(),
        createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(),
        createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass()
    };

    float3 positionWS[vertexCount * billboardCount];

	// This is used to calculate the current V position of our TexCoords.
	// We know the U position, because even vertices (0, 2, 4, ...) always have X = 0
	// And uneven vertices (1, 3, 5, ...) always have X = 1
    float VOffset = 1;

	// Wind
    float windCoEff = 0;
    
    float size = 1;
    for (uint j = 0; j < billboardCount; j++)
    {
        v[j * 4 + 0].Uv = TexCoord[0];
        v[j * 4 + 1].Uv = TexCoord[1];
        v[j * 4 + 2].Uv = TexCoord[2];
        v[j * 4 + 3].Uv = TexCoord[3];
        v[j * 4 + 0].Position = float4(root.xyz - size * float3(1, 0, 0) - size * float3(0, 1, 0), 1.0f);
        v[j * 4 + 1].Position = float4(root.xyz - size * float3(1, 0, 0) + size * float3(0, 1, 0), 1.0f);
        v[j * 4 + 2].Position = float4(root.xyz + size * float3(1, 0, 0) - size * float3(0, 1, 0), 1.0f);
        v[j * 4 + 3].Position = float4(root.xyz + size * float3(1, 0, 0) + size * float3(0, 1, 0), 1.0f);
        for (uint i = 0; i < vertexCount; i++)
        {
		    // Fake creation of the normal. Pointing downwards on the bottom. Pointing upwards on the top. And then interpolating in between.
            v[j * 4 + i].Normal = normalize(float4(0, 1, 0, 1)).xyz;
            
		    // First rotate (translate to origin)
            v[j * 4 + i].Position = float4(v[j * 4 + i].Position.x - root.x, v[j * 4 + i].Position.y - root.y, v[j * 4 + i].Position.z - root.z, 1);
        
            v[j * 4 + i].Position = float4(mul(v[j * 4 + i].Position.xyz, rotationMatrix), 1);
            v[j * 4 + i].Position = float4(mul(v[j * 4 + i].Position.xyz, rot[j]), 1);
            v[j * 4 + i].Position = float4(v[j * 4 + i].Position.x + root.x, v[j * 4 + i].Position.y + root.y, v[j * 4 + i].Position.z + root.z, 1);

		    // Wind
            float2 windVec = WindVector + (WindAccel * Time);
            windVec.x += (sin(Time.x + root.x / 25) + sin((Time.x + root.x / 15) + 50)) * 0.5;
            windVec.y += cos(Time.x + root.z / 80);
            windVec *= lerp(0.7, 1.0, 1.0 - random);

		    // Oscillate wind
            float sinSkewCoeff = random;
            float oscillationStrength = 0.5f * size;
            float lerpCoeff = (sin(oscillationStrength * Time.x + sinSkewCoeff) + 1.0) / 2;
            float2 leftWindBound = windVec * (1.0 - kOscillateDelta);
            float2 rightWindBound = windVec * (1.0 + kOscillateDelta);
            windVec = lerp(leftWindBound, rightWindBound, lerpCoeff);

		    // Randomize wind by adding a random wind vector
            float randAngle = lerp(-3.14, 3.14, random);
            float randMagnitude = lerp(0, 1.0, random);
            float2 randWindDir = float2(sin(randAngle), cos(randAngle));
            windVec += randWindDir * randMagnitude;

            float windForce = length(windVec);

		    // Calculate final vertex position based on wind
            v[j * 4 + i].Position.xz += windVec.xy * windCoEff;
            v[j * 4 + i].Position.y -= windForce * windCoEff * 0.1f;
            v[j * 4 + i].Position.y += 1;
            v[j * 4 + i].wPosition = v[j * 4 + i].Position;
            positionWS[j * 4 + i] = mul(v[j * 4 + i].Position, World).xyz;

		    // Calculate output
            v[j * 4 + i].Position = mul(v[j * 4 + i].Position, World);
            v[j * 4 + i].ShadowTransform = mul(v[j * 4 + i].Position, ShadowTransform);
            v[j * 4 + i].Position = mul(v[j * 4 + i].Position, View);
            v[j * 4 + i].Position = mul(v[j * 4 + i].Position, Projection);

            v[j * 4 + i].VertexToLight = normalize(LightPosition - positionWS[i].xyz);
            v[j * 4 + i].VertexToCamera = normalize(ViewPosition - positionWS[i].xyz);

            v[j * 4 + i].Random = random;
        
            if (i % 2 == 0)
                windCoEff = VOffset;
            else
                windCoEff = 0;
        }
        
	    //[unroll]
        for (uint p = 0; p < vertexCount; p++)
        {
            stream.Append(v[j * 4 + p]);
        }
        stream.RestartStrip();
    }
}

struct GeometryOutput_Flower
{
    float4 Position : SV_POSITION;
    float3 wPosition : POSITION1;
    float3 LookDir : DIR0;
    float2 Uv : UV0;
    float4 ShadowTransform : UV1;
    float3 Normal : normal0;
};

GeometryOutput_Flower createGEO_OUT_Flower()
{
    GeometryOutput_Flower output;
	
    float4 Position = float4(0, 0, 0, 0);
    float3 wPosition = float3(0, 0, 0);
    float3 LookDir = float3(0, 0, 0);
    float2 Uv = float2(0, 0);
    float4 ShadowTransform = float4(0, 0, 0, 0);
    float3 Normal = float3(0, 0, 0);

    return output;
}


// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //

sampler TextureSampler = sampler_state
{
    AddressU = Wrap;
    AddressV = Wrap;
    MaxAnisotropy = 16;
    MaxLOD = 2.0f;
};

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

Texture2DArray Map;
int TextureNumber;
int MaxTextureCount;

float4 PS_Grass(in GeometryOutput input) : SV_TARGET
{
    float4 textureColor = Map.Sample(TextureSampler, float3(input.Uv, TextureNumber % MaxTextureCount));
    clip(textureColor.a - 0.4f);

	// Phong
    float3 r = normalize(reflect(input.VertexToLight.xyz, input.Normal.xyz));
    float shininess = 100;
    
    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowTransform, input.wPosition.xyz);
    
    float ambientLight = 0.3f;
    float diffuseLight = saturate(dot(input.VertexToLight, input.Normal.xyz)) * shadow;
    float specularLight = saturate(dot(-input.VertexToCamera, r)) * shadow;
    specularLight = saturate(pow(specularLight, shininess));
	
    float light = ambientLight + (diffuseLight * 1.55) + (specularLight * 0.5);

    float3 grassColorHSV = { 0.17 + (input.Random / 20), 0.17 + (input.Random / 20), 1 };
    float3 grassColorRGB = HSVtoRGB(grassColorHSV);

    return float4(light * textureColor.rgb * grassColorRGB, textureColor.a);
}

PixelOutPut PS_Grass_GB(in GeometryOutput input)
{
    PixelOutPut output;

    float4 textureColor = Map.Sample(TextureSampler, float3(input.Uv, TextureNumber % MaxTextureCount));
    clip(textureColor.a - 0.9f);

	// Phong
    float3 r = normalize(reflect(input.VertexToLight.xyz, input.Normal.xyz));
    float shininess = 100;
    
    //float shadow = float3(1.0f, 1.0f, 1.0f);
    //shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowTransform, input.wPosition.xyz);
    
    float ambientLight = 0.8f;
    float diffuseLight = saturate(dot(input.VertexToLight, input.Normal.xyz));
    float specularLight = saturate(dot(-input.VertexToCamera, r));
    specularLight = saturate(pow(specularLight, shininess));
	
    float light = ambientLight + (diffuseLight * 1.55) + (specularLight * 0.5);

    float3 grassColorHSV = { 0.17 + (input.Random / 20), 0.17 + (input.Random / 20), 1 };
    float3 grassColorRGB = HSVtoRGB(grassColorHSV);

    textureColor = float4(light * textureColor.rgb * grassColorRGB, textureColor.a);
    
    //textureColor.rgb *= Diffuse.rgb;
    textureColor.r += 0.1f;
    textureColor.g += 0.1f;
    textureColor.b += 0.1f;

    output.Position = input.wPosition;
    output.Normal = float4(input.Normal.xyz, 1.0f);
    output.Diffuse = float4(textureColor.rgb, 1.0f);
    output.Specular = float4(0, 0, 0, 1.0f);

    return output;
}


// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //

//  Grass
technique11 T0
{
    //  LOD1
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Grass()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Grass()));
        SetPixelShader(CompileShader(ps_5_0, PS_Grass()));

        SetRasterizerState(Rasterizer);
    }
}

technique11 T2
{
    //  LOD1
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Grass()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Grass()));
        SetPixelShader(CompileShader(ps_5_0, PS_Grass_GB()));

        SetRasterizerState(Rasterizer);
    }
}
