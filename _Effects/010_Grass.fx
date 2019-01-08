#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //
float MinWidth;
float MinHeight;

float2 TimeVector;
float2 WindVector;

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

struct GeometryInput
{
    float4 Position : POSITION0;
    float3 Normal : NORMAL0;
    //int Cull : Cull0;
};

//bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
//{
//    float3 n = abs(plane.xyz);
	
//	// This is always positive.
//    float r = dot(extents, n);
	
//	// signed distance from center point to plane.
//    float s = dot(float4(center, 1.0f), plane);
	
//	// If the center point of the box is a distance of e or more behind the
//	// plane (in which case s is negative since it is behind the plane),
//	// then the box is completely in the negative half space of the plane.
//    return (s + r) < 0.0f;
//}

//bool AabbOutsideFrustumTest(float3 center, float3 extents)
//{
//    [unroll]
//    for (int i = 0; i < 6; ++i)
//    {
//		// If the box is completely behind any of the frustum planes
//		// then it is outside the frustum.
//        [flatten]
//        if (AabbBehindPlaneTest(center, extents, FrustumPlanes[i]))
//        {
//            return true;
//        }
//    }
	
//    return false;
//}

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
    float minHeight = 3.5f;
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

    return output;
}

[maxvertexcount(4)]
void GS_Grass(point GeometryInput points[1], inout TriangleStream<GeometryOutput> stream)
{
    //if (points[0].Cull == 1)
    //    return;

    float4 root = points[0].Position;

    LightPosition = -LightDirection * 200;
    
	// Generate a random number between 0.0 to 1.0 by using the root position (which is randomized by the CPU)
    float random = sin(kHalfPi * frac(root.x) + kHalfPi * frac(root.z));
    float randomRotation = random;

    float cameraDistance = length(ViewPosition.xz - root.xz);

	// Properties of the grass blade
    float minHeight = 3.5;
    float minWidth = 0.02f + (cameraDistance * 0.0001);
    float sizeX = minWidth + (random / 50);
    float sizeY = minHeight + (random / 5);
    
	// Rotate in Z-axis
    float3x3 rotationMatrix =
    {
        cos(randomRotation), 0, sin(randomRotation),
		0, 1, 0,
		-sin(randomRotation), 0, cos(randomRotation)
    };

	/////////////////////////////////
	// Generating vertices
	/////////////////////////////////

    const uint vertexCount = 4;
    GeometryOutput v[vertexCount] =
    {
        createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass(), createGEO_OUT_Grass()
    };

    float3 positionWS[vertexCount];

	// This is used to calculate the current V position of our TexCoords.
	// We know the U position, because even vertices (0, 2, 4, ...) always have X = 0
	// And uneven vertices (1, 3, 5, ...) always have X = 1
    float currentV = 1;
    float VOffset = 1 / ((vertexCount / 2) - 1);
    float currentNormalY = 0;
    float currentHeightOffset = sqrt(sizeY);
    float currentVertexHeight = 0;

	// Wind
    float windCoEff = 0;

	// We don't want to interpolate linearly for the normals. The bottom vertex should be 0, top vertex should be 1.
	// If we interpolate linearly and we have 4 vertices, we get 0, 0.33, 0.66, 1. 
	// Using pow, we can adjust the curve so that we get lower values on the bottom and higher values on the top.
    float steepnessFactor = 1.75;
    

    //  중점으로부터 4개의 정점을 구하기 위해 up, forward, right 벡터를 구함
    float3 look = ViewPosition - points[0].Position.xyz; //  카메라를 바라보도록 계산
    look = saturate(look);
    
    matrix Rx = 0;
    Rx._22_23_32_33 = float4(cos(look.x), sin(look.x), -sin(look.x), cos(look.x));
    //Rx._22 = cos(look.x);
    //Rx._23 = sin(look.x);
    //Rx._32 = -sin(look.x);
    //Rx._33 = cos(look.x);
    float3x3 Ry = 0.0f;
    Ry._22 = 1.0f;
    Ry._11_13_31_33 = float4(cos(look.y), sin(look.y), -sin(look.y), cos(look.y));
    //Ry._11 = cos(look.y);
    //Ry._13 = -sin(look.y);
    //Ry._31 = sin(look.y);
    //Ry._33 = cos(look.y);
    matrix Rz = 0;
    Rz._11_12_21_22 = float4(cos(look.z), sin(look.z), -sin(look.z), cos(look.z));

    //matrix R = Rx * Ry * Rz;
    //
    //float x = asin(-R._32);
    //float y = atan2(R._31, R._33);
    //float z = atan2(R._12, R._22);

	
	// Transform into projection space and calculate vectors needed for light calculation
	[unroll]
    for (uint i = 0; i < vertexCount; i++)
    {
		// Fake creation of the normal. Pointing downwards on the bottom. Pointing upwards on the top. And then interpolating in between.
        v[i].Normal = normalize(float4(0, pow(currentNormalY, steepnessFactor), 0, 1)).xyz;

		// Creating vertices and calculating Texcoords (UV)
		// Vertices start at the bottom and go up. v(0) and v(1) are left bottom and right bottom.
        if (i % 2 == 0)
        { // 0, 2, 4
            v[i].Position = float4(root.x - sizeX, root.y + currentVertexHeight, root.z, 1);
            v[i].Uv = float2(0, currentV);
        }
        else
        { // 1, 3, 5
            v[i].Position = float4(root.x + sizeX, root.y + currentVertexHeight, root.z, 1);
            v[i].Uv = float2(1, currentV);
        }

        //v[i].Position = billboard[i];
        //v[i].Position = float4(mul(v[i].Position.xyz, rotationMatrix), 1);
		// First rotate (translate to origin)
        v[i].Position = float4(v[i].Position.x - root.x, v[i].Position.y - root.y, v[i].Position.z - root.z, 1);
        
        v[i].Position = float4(mul(v[i].Position.xyz, rotationMatrix), 1);
        v[i].Position = float4(mul(v[i].Position.xyz, Ry), 1);
        v[i].Position = float4(v[i].Position.x + root.x, v[i].Position.y + root.y, v[i].Position.z + root.z, 1);

		// Wind
        float2 windVec = WindVector;
        windVec.x += (sin(Time.x + root.x / 25) + sin((Time.x + root.x / 15) + 50)) * 0.5;
        windVec.y += cos(Time.x + root.z / 80);
        windVec *= lerp(0.7, 1.0, 1.0 - random);

		// Oscillate wind
        float sinSkewCoeff = random;
        float oscillationStrength = 2.5f;
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
        v[i].Position.xz += windVec.xy * windCoEff;
        v[i].Position.y -= windForce * windCoEff * 0.8f;
        v[i].wPosition = v[i].Position;
        positionWS[i] = mul(v[i].Position, World).xyz;

		// Calculate output
        v[i].Position = mul(v[i].Position, World);
        v[i].ShadowTransform = mul(v[i].Position, ShadowTransform);
        v[i].Position = mul(v[i].Position, View);
        v[i].Position = mul(v[i].Position, Projection);

        v[i].VertexToLight = normalize(LightPosition - positionWS[i].xyz);
        v[i].VertexToCamera = normalize(ViewPosition - positionWS[i].xyz);

        v[i].Random = random;
        

        if (i % 2 != 0)
        {
			// General
            currentV -= VOffset;
            currentNormalY += VOffset * 2;

			// Height
            currentHeightOffset -= VOffset;
            float currentHeight = sizeY - (currentHeightOffset * currentHeightOffset);
            currentVertexHeight = currentHeight;

			// Wind
            windCoEff += VOffset; // TODO: Check these values
        }
    }

	// Connect the vertices
	[unroll]
    for (uint p = 0; p < (vertexCount - 2); p++)
    {
        stream.Append(v[p]);
        stream.Append(v[p + 2]);
        stream.Append(v[p + 1]);
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

static const float2 TexCoord[4] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
};

[maxvertexcount(4)]
void GS_Flower(point GeometryInput input[1], inout TriangleStream<GeometryOutput_Flower> stream)
{
    // Generate a random number between 0.0 to 1.0 by using the root position (which is randomized by the CPU)
    float random = sin(kHalfPi * frac(input[0].Position.x) + kHalfPi * frac(input[0].Position.z));

    // Wind
    float2 windVec = WindVector;
    windVec.x += (sin(Time.x + input[0].Position.x / 25) + sin((Time.x + input[0].Position.x / 15) + 50)) * 0.5;
    windVec.y += cos(Time.x + input[0].Position.z / 80);
    windVec *= lerp(0.7, 1.0, 1.0 - random);

	// Oscillate wind
    float sinSkewCoeff = random;
    float oscillationStrength = 2.5f;
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
    input[0].Position.xz += windVec.xy;
    input[0].Position.y -= windForce * 0.8f;
    

    float2 size = float2(0.5f, 0.5f);
    //  중점으로부터 4개의 정점을 구하기 위해 Size를 반으로 나눔
    float halfWidth = 0.5f * size.x;
    float halfHeight = 0.5f * size.y;

    //  중점으로부터 4개의 정점을 구하기 위해 up, forward, right 벡터를 구함
    float3 look = ViewPosition - input[0].Position.xyz; //  카메라를 바라보도록 계산
    look.y = 0.0f;
    look = normalize(look);

    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, look));
    
    //  up과 right, halfWidth, halfHeight를 사용해서 4개의 정점을 구함
    float4 v[4];
    v[0] = float4(input[0].Position.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].Position.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].Position.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].Position.xyz - halfWidth * right + halfHeight * up, 1.0f);
    
    GeometryOutput_Flower output = createGEO_OUT_Flower();
    
    float VOffset = 1;
    float currentNormalY = 0;
    
	// We don't want to interpolate linearly for the normals. The bottom vertex should be 0, top vertex should be 1.
	// If we interpolate linearly and we have 4 vertices, we get 0, 0.33, 0.66, 1. 
	// Using pow, we can adjust the curve so that we get lower values on the bottom and higher values on the top.
    float steepnessFactor = 1.75;
    
    //  GS도 VS와 마찬가지로 3D 공간상에 출력하기 위해서는
    //  WVP변환이 필요하다.
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        output.Position = mul(v[i], World);
        output.ShadowTransform = mul(output.Position, ShadowTransform);
        output.Position = mul(output.Position, View);
        output.Position = mul(output.Position, Projection);

        output.wPosition = mul(v[i], World).xyz;

        output.Uv = TexCoord[i];
        output.LookDir = look;

        output.Normal = normalize(float4(0, pow(currentNormalY, steepnessFactor), 0, 1)).xyz;
        if (i % 2 != 0)
        {
            currentNormalY += VOffset * 2;
        }

        stream.Append(output);
    }
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
Texture2D GrassMap;
Texture2D GrassAlphaMap1;
Texture2D GrassAlphaMap2;

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

float4 PS_Grass(in GeometryOutput input) : SV_TARGET
{
    float4 textureColor = DiffuseMap.Sample(TextureSampler, input.Uv);
    float4 alphaColor = NormalMap.Sample(TextureSampler, input.Uv);
    float4 alphaColor2 = SpecularMap.Sample(TextureSampler, input.Uv);
    
	// Phong
    float3 r = normalize(reflect(input.VertexToLight.xyz, input.Normal.xyz));
    float shininess = 100;
    
    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowTransform);
    
    float ambientLight = 0.1;
    float diffuseLight = saturate(dot(input.VertexToLight, input.Normal.xyz)) * shadow;
    float specularLight = saturate(dot(-input.VertexToCamera, r)) * shadow;
    specularLight = saturate(pow(specularLight, shininess));
	
    float light = ambientLight + (diffuseLight * 1.55) + (specularLight * 0.5);
	
    float3 grassColorHSV = { 0.17 + (input.Random / 20), 1, 1 };
    float3 grassColorRGB = HSVtoRGB(grassColorHSV);

    float3 lightColor = float3(1.0, 0.8, 0.8);

    if (alphaColor.g <= 0.8)
    {
        alphaColor.g = 0;
    }

    return float4(light * textureColor.rgb * grassColorRGB, alphaColor2.g);
}

Texture2D FlowerMap;
float4 PS_Flower(in GeometryOutput_Flower input) : SV_TARGET
{
    float3 toEye = normalize(ViewPosition - input.wPosition);

    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);

    float shadow = float3(1.0f, 1.0f, 1.0f);
    shadow = CalcShadowFactor(samShadow, ShadowMap, input.ShadowTransform);

    Material m = { Ambient, Diffuse, Specular, Shininess };
    DirectionalLight l = { LightAmbient, LightDiffuse, LightSpecular, LightDirection };


    float4 A, D, S;
    float3 normal = float3(0, 1, 0);
    ComputeDirectionalLight(m, l, SunColor, normal, toEye, A, D, S);
    ambient += A;
    diffuse += shadow * D;
    specular += shadow * S;

    float4 flower = DetailMap.Sample(TextureSampler, input.Uv);
    
    float4 color = float4(ambient + diffuse + specular, 1.0f);
    color *= flower;

    return color;
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
        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

//  Flower
technique11 T1
{
    
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Flower()));
        SetGeometryShader(CompileShader(gs_5_0, GS_Flower()));
        SetPixelShader(CompileShader(ps_5_0, PS_Flower()));

        SetRasterizerState(Rasterizer);
        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}