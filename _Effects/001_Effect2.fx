#include "000_Header.fx"

//  --------------------------------------------------------------------------- //
//  States
//  --------------------------------------------------------------------------- //
RasterizerState FillMode
{
    //  D3D11_RASTERIZER_DESC �ȿ� �ִ� �̸��̶� ����
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
    
    return output; // ��ȯ���� �ȼ��� ��ġ
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
        //  �ʿ���� ��� : SetVertexShader(NULL)
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        
        //  DC���ٰ� �����ϴ� �ű� ������ �ѹ� �����ϸ� �������� �״�� ����� ���·� �����ִ�.
        //  ���߿� ���� State ���� �ϴ°� ���� �� �ֽŴٰ� ����
        SetRasterizerState(FillMode);
    }
}