#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //
RasterizerState FillMode
{
    //  D3D11_RASTERIZER_DESC 안에 있는 이름이랑 같음
    FillMode = Wireframe;
};

//  --------------------------------------------------------------------------- //
//  Vertex Shader
//  --------------------------------------------------------------------------- //
struct VertexInput
{
    float4 Position : POSITION0;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    return output; // 반환값이 픽셀의 위치
}

//  --------------------------------------------------------------------------- //
//  Pixel Shader
//  --------------------------------------------------------------------------- //
float4 PS(VertexOutput input) : SV_TARGET
{
    return Color;
}

//  --------------------------------------------------------------------------- //
//  Technique & Pass
//  --------------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        //  필요없는 경우 : SetVertexShader(NULL)
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        
        //  DC에다가 설정하는 거기 때문에 한번 설정하면 다음에도 그대로 적용된 상태로 남아있다.
        //  나중에 쌤이 State 구분 하는거 수정 해 주신다고 했음
        SetRasterizerState(FillMode);
    }
}