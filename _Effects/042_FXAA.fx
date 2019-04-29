#define FXAA_PC 1  
#define FXAA_HLSL_5 1 
#define FXAA_QUALITY__PRESET  39
#define FXAA_GREEN_AS_LUMA 1
#include "FXAA3_11.h"

// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //
float4 RCPFrame;
bool Use;

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
};

VertexOutput VS(uint id : SV_VertexID)
{
    //  W : 동촤 -> 0 : 방향 , 1 : 위치
    VertexOutput output;

    output.Uv = float2((id << 1) & 2, id & 2);
    
    output.Position.x = output.Uv.x * 2 - 1;
    output.Position.y = output.Uv.y * -2 + 1;
    output.Position.z = 0.0f;
    output.Position.w = 1.0f;

    return output; // 반환값이 픽셀의 위치
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
SamplerState InputSampler
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
    MipLodBias = 0.0f;
    MaxAnisotropy = 1;
    ComparisonFunc = ALWAYS;
};

SamplerState Sampler;

Texture2D InputTexture;

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 color = 1;
    if (Use == true)
    {
        FxaaTex InputFXAATex = { InputSampler, InputTexture };
        
        color = FxaaPixelShader(
        input.Uv.xy, // FxaaFloat2 pos,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsolePosPos,
        InputFXAATex, // FxaaTex tex,
        InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegOne,
        InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        RCPFrame.xy, // FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75f, // FxaaFloat fxaaQualitySubpix,
        0.166f, // FxaaFloat fxaaQualityEdgeThreshold,
        0.0833f, // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f) // FxaaFloat fxaaConsole360ConstDir,
    );
    }  
    else
    {
        int3 sampleIndices = int3(input.Position.xy, 0);
        color = InputTexture.Load(sampleIndices);
    }

    return color;
}

// --------------------------------------------------------------------- //
//  States
// --------------------------------------------------------------------- //
DepthStencilState Depth
{
    DepthEnable = false;
};
// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetDepthStencilState(Depth, 0);
    }
}