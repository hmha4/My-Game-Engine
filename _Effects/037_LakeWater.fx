#include "000_Header.fx"

// ------------------------------------------------------------------------- //
//  Contants
// ------------------------------------------------------------------------- //
matrix WindDirection;

// ------------------------------------------------------------------------- //
//  Variables
// ------------------------------------------------------------------------- //
matrix ReflectionView;

//  Bump map
float WaveLength;
float WaveHeight;

float WaterSpeed;
float WindForce;
float DrawMode;

//  Fresnel
int FresnelMode;

//  Specular
float SpecPerturb;
float SpecPower;

//  Dull blend
float DullBlendFactor;

// ------------------------------------------------------------------------- //
//  Texture & Samplers
// ------------------------------------------------------------------------- //
Texture2D ReflectionMap;
Texture2D RefractionMap;
Texture2D WaterBumpMap;
Texture2D ScreenDepth;
SamplerState ReflectionSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};
SamplerState RefractionSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};


// ------------------------------------------------------------------------- //
//  Vertex Shader
// ------------------------------------------------------------------------- //

struct VertexOutput
{
    float4 Position : SV_Position0;
    float4 ReflectionMapSamplingPos : Uv0;
    float2 BumpMapSamplingPos : Uv1;
    float4 RefractionMapSamplingPos : Uv2;
    float4 wPosition : Uv3;
};

VertexOutput VS(VertexTexture input)
{
    VertexOutput output;

    matrix VP = mul(View, Projection); //  View Projection
    matrix WVP = mul(World, VP); //  World View Projection
    matrix RVP = mul(ReflectionView, Projection); //  Reflection View Projection
    matrix WRVP = mul(World, RVP); //  World Reflection View Projection

    output.Position = mul(input.Position, WVP);
    output.wPosition = mul(input.Position, World);
    output.ReflectionMapSamplingPos = mul(input.Position, WRVP);
    output.RefractionMapSamplingPos = mul(input.Position, WVP);
    

    float4 absoluteTexCoords = float4(input.Uv, 0, 1);
    float4 rotatedTexCoords = mul(absoluteTexCoords, WindDirection);
    float2 moveVector = float2(0, 1);

    //  move the water
    output.BumpMapSamplingPos = rotatedTexCoords.xy / WaveLength + (Time * WaterSpeed) * WindForce * moveVector.xy;

    return output;
}

void SampleColor(inout float4 color, VertexOutput input)
{
    float2 projectedTexCoords;
    projectedTexCoords.x = input.ReflectionMapSamplingPos.x / input.ReflectionMapSamplingPos.w / 2.0f + 0.5f;
    projectedTexCoords.y = -input.ReflectionMapSamplingPos.y / input.ReflectionMapSamplingPos.w / 2.0f + 0.5f;

    //  Sampling bump map
    float4 bumpColor = WaterBumpMap.Sample(ReflectionSampler, input.BumpMapSamplingPos);

    //  Perturbating the color
    float2 perturbation = WaveHeight * (bumpColor.rg - 0.5f);

    //  Final texture coordinates
    float2 perturbatedTexCoords = projectedTexCoords + perturbation;
    float4 reflectiveColor = ReflectionMap.Sample(ReflectionSampler, perturbatedTexCoords);

    //  alpha is set to 1 except when the reflection map does not have a valid 
	//  value (color is completely black)
    if (reflectiveColor.r + reflectiveColor.b + reflectiveColor.g < 0.1f)
    {
        reflectiveColor.a = 1.0f;
    }
    else
    {
        reflectiveColor.a = 1.0f;
    }

    float2 projectedRefrTexCoords;
    projectedRefrTexCoords.x = input.RefractionMapSamplingPos.x / input.RefractionMapSamplingPos.w / 2.0f + 0.5f;
    projectedRefrTexCoords.y = -input.RefractionMapSamplingPos.y / input.RefractionMapSamplingPos.w / 2.0f + 0.5f;
    float2 perturbatedRefrTexCoords = projectedRefrTexCoords + perturbation;
    float4 refractiveColor = RefractionMap.Sample(RefractionSampler, perturbatedRefrTexCoords);

    //  alpha is set to 1 except when the refraction map does not have a valid 
	//  value (color is completely black)
    if (refractiveColor.r + refractiveColor.b + refractiveColor.g < 0.1f)
    {
        refractiveColor.a = 1.0f;
    }
    else
    {
        refractiveColor.a = 1.0f;
    }

    float3 eyeVector = normalize(ViewPosition - input.wPosition.xyz);
    float3 normalVector = float3(0, 1, 0);
   

    /////////////////////////////////////////////////
    // FRESNEL TERM APPROXIMATION
    /////////////////////////////////////////////////
    float fresnelTerm = (float) 0;
    if (FresnelMode == 1)
    {
        fresnelTerm = 0.02 + 0.97f * pow((1 - dot(eyeVector, normalVector)), 5);
    }
    else /*if (FresnelMode == 0)*/
    {
        fresnelTerm = 1 - dot(eyeVector, normalVector) * 1.3f;
    }
	//else if (FresnelMode == 2)
	//{
	//	float fangle = 1.0f+dot(eyeVector, normalVector);
	//	fangle = pow(fangle,5);  
    //  fresnelTerm = fangle*50;
	//	fresnelTerm = 1/fangle;
    //}

	//fresnelTerm = (1/pow((fresnelTerm+1.0f),5))+0.2f;

    //  Hardness
    fresnelTerm = fresnelTerm * DrawMode;

    //  Clamp value between 0 and 1
    fresnelTerm = fresnelTerm < 0 ? 0 : fresnelTerm;
    fresnelTerm = fresnelTerm > 1 ? 1 : fresnelTerm;

    //  Create the combined color
    float4 combinedColor = refractiveColor * (1 - fresnelTerm) * refractiveColor.a * reflectiveColor.a + reflectiveColor * fresnelTerm * reflectiveColor.a * refractiveColor.a;
    
    float3 eye = ViewPosition - input.wPosition.xyz;
    float distanceToEye = length(eye);

    float fogFactor = saturate((distanceToEye - 300) / 200);
    
    color = lerp(color, SunColor, fogFactor);

    /////////////////////////////////////////////////
	// WATER COLORING
	/////////////////////////////////////////////////
    //float4 color = 0;
    
    //  Add some dull color to make the water darker then the reflected picture
    color = DullBlendFactor * combinedColor * float4(0.95f, 1.00f, 1.05f, 1.0f);
    

    /////////////////////////////////////////////////
	// Specular Highlights
	/////////////////////////////////////////////////
    float4 speccolor;
    float3 tangent = float3(1, 0, 0);
    normalVector = NormalSampleToWorldSpace(bumpColor.rgb, normalVector, tangent);
    //SpecularLighting(speccolor, normalVector, eyeVector);

    float3 lightSourceDir = -LightDirection;
    
    float3 halfvec = normalize(eyeVector + lightSourceDir + float3(perturbation.x * SpecPerturb, perturbation.y * SpecPerturb, 0));
	
    float temp = 0;
    temp = pow(dot(halfvec, normalVector), SpecPower);
	
    speccolor = SunColor;
	
    speccolor = speccolor * temp;
    
    speccolor = float4(speccolor.x * speccolor.w, speccolor.y * speccolor.w, speccolor.z * speccolor.w, 0);

    color = color + speccolor;
}

float4 PS(VertexOutput input) : SV_Target
{
    float4 color = 0;
    float sceneZ = ScreenDepth.Load(int3(input.Position.xy, 0));

    if (dot(sceneZ, 1.0f) != 0)
    {
        float4 clipPos = mul(input.wPosition, World);
        clipPos = mul(clipPos, View);
        clipPos = mul(clipPos, Projection);

        clipPos.z /= clipPos.w;
        if (clipPos.z < sceneZ)
        {
            SampleColor(color, input);
        }
    }
    else
    {
        SampleColor(color, input);
    }
    
    return color;
}


BlendState Blend
{
    AlphaToCoverageEnable = false;

    BlendEnable[0] = true;
    RenderTargetWriteMask[0] = 1 | 2 | 4 | 8; // Write all colors R | G | B | A

    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
};

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

