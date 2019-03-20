#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
Texture2D WorldData;

struct VertexInput
{
    float4  Position : Position0;
    uint Id : SV_VertexID;  //  CS 처럼 DispatchID 역할

    float4 Color : Instance0;
    uint InstanceID : SV_InstanceID0;
};

struct VertexOutput
{
    float4 Position : SV_Position0;
    float4 Color : Color0;
    uint Id : VertexID0;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    //  input.InstanceID * 4 : 하나의 매트릭스
    //  (input.InstanceID * 4) + 3 : 하나의 매트릭스에 대한 위치 값
    float3 position = 0;
    position.x = WorldData.Load(int3(3, input.InstanceID, 0)).x;
    position.y = WorldData.Load(int3(7, input.InstanceID, 0)).y;
    position.z = WorldData.Load(int3(11, input.InstanceID, 0)).z;
    
    output.Position.xyz = input.Position.xyz + position;
    output.Position.w = 1.0f;

    output.Color = input.Color;
    output.Id = input.Id;

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //

float4 PS(VertexOutput input) : SV_TARGET
{
    return input.Color;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}