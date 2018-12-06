#include "000_Header.fx"

// --------------------------------------------------------------------- //
//  Global Variables
// --------------------------------------------------------------------- //
float Ratio = 20.0f;

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //
struct VertexOutput
{
    float4 Position : Position0;
};

VertexOutput VS(Vertex input)
{
    VertexOutput output;

    output.Position = input.Position;

    return output;
}

// --------------------------------------------------------------------- //
//  Hull Shader
// --------------------------------------------------------------------- //
struct ConstantOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsidetessFactor;
};

int ComputeAmount(float3 position)
{
    float dist = distance(position, ViewPosition);
    float s = saturate((dist - Ratio) / (100.0f - Ratio));

    return (int) lerp(64, 1, s);
}

ConstantOutput HS_Constant(InputPatch<VertexOutput, 4> input, uint patchId : SV_PrimitiveId)
{
    ConstantOutput output;

    float3 center = 0.0f;
    center = (input[0].Position.xyz + input[1].Position.xyz) * 0.5f;
    output.Edges[0] = ComputeAmount(center);
    
    center = (input[1].Position.xyz + input[3].Position.xyz) * 0.5f;
    output.Edges[1] = ComputeAmount(center);

    center = (input[2].Position.xyz + input[3].Position.xyz) * 0.5f;
    output.Edges[2] = ComputeAmount(center);

    center = (input[0].Position.xyz + input[2].Position.xyz) * 0.5f;
    output.Edges[3] = ComputeAmount(center);
    
    center = (input[0].Position + input[1].Position + input[2].Position + input[3].Position) * 0.25f;
    output.Inside[0] = ComputeAmount(center);
    output.Inside[1] = output.Inside[0];

    return output;
}

struct HullOutput
{
    float4 Position : SV_Position0;
};

//  line
//  line_adj
//  tri
//  tri_adj
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
//[maxtessfactor]
HullOutput HS(InputPatch<VertexOutput, 4> input, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveId)
{
    HullOutput output;
    output.Position = input[pointID].Position;

    return output;
}

// --------------------------------------------------------------------- //
//  Domain Shader
// --------------------------------------------------------------------- //
struct DomainOutput
{
    float4 Position : SV_Position0;
};

[domain("quad")]
DomainOutput DS(ConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HullOutput, 4> patch)
{
    DomainOutput output;
    
    float3 v1 = lerp(patch[0].Position.xyz, patch[1].Position.xyz, 1 - uv.y);
    float3 v2 = lerp(patch[2].Position.xyz, patch[3].Position.xyz, 1 - uv.y);
    float3 position = lerp(v1, v2, uv.x);

    output.Position = mul(float4(position, 1), World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    return output;
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
float4 PS(DomainOutput input) : SV_TARGET
{
    return float4(0, 0, 1, 1);
}

// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
RasterizerState FillMode
{
    FillMode = Wireframe;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(FillMode);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

}