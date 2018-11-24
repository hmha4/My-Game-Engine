
//	1. IA : Input Assembly( 기초 데이터 세팅 )
//	2. VS : Vertex Shader( Culling:vertex단위 )
//	3. HS : Hul Shading
//	4. DS : Domain Shading
//	5. GS : Geometry Shader
//		- RS : Rasterizer State( 3D 에서 2D 로 변환, Backface Culling )
//		- PS : Pixel Shader( Clipping:pixel단위 )
//		- MR : Multi Rendertarget
//		- OM : Ouput Merger( 출력을 통합, blendState 이 단계에서 이루어짐 )
//	6. SO : Stream Output

//  --------------------------------------------------------------------------- //
//  Constant Buffer
//  --------------------------------------------------------------------------- //
cbuffer CB_PerFrame
{
    matrix View;
    float3 ViewDirection;

    float Time;
    float3 ViewPosition;
};

cbuffer CB_Projection
{
    matrix Projection;
};


cbuffer CB_World
{
    matrix World;
};

cbuffer CB_Light
{
    float3 LightDirection;
    float3 LightColor;
}

cbuffer CB_Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float  Shininess;
}


// --------------------------------------------------------------------- //
//  Textures
// --------------------------------------------------------------------- //
Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;
Texture2D DetailMap;


//  --------------------------------------------------------------------------- //
//  Vertex Layouts
//  --------------------------------------------------------------------------- //
struct Vertex
{
    float4 Position : POSITION0;
};

struct VertexColor
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
};

struct VertexColorNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float3 Normal : NORMAL0;
};

struct VertexTexture
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
};

struct VertexTextureNormal
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
};

struct VertexTextureNormalTangent
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

struct VertexTextureNormalTangentBlend
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;
};

struct VertexColorTextureNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
};

//  --------------------------------------------------------------------------- //
//  Bones
//  --------------------------------------------------------------------------- //
int BoneIndex;
cbuffer CB_Bones
{
    matrix Bones[128];
};

matrix BoneWorld()
{
    return Bones[BoneIndex];
}

matrix SkinWorld(float4 blendIndices, float4 blendWeights)
{
    float4x4 transform = 0;
    transform += mul(blendWeights.x, Bones[(uint) blendIndices.x]);
    transform += mul(blendWeights.y, Bones[(uint) blendIndices.y]);
    transform += mul(blendWeights.z, Bones[(uint) blendIndices.z]);
    transform += mul(blendWeights.w, Bones[(uint) blendIndices.w]);

    return transform;
}