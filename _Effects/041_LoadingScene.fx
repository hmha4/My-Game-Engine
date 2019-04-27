// --------------------------------------------------------------------- //
//  Constant Buffers
// --------------------------------------------------------------------- //
matrix World;
matrix View;
matrix Projection;

float AlphaValue;

// --------------------------------------------------------------------- //
//  Vertex Shader
// --------------------------------------------------------------------- //

struct VertexTexture
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
};

VertexOutput VS(VertexTexture input)
{
    //  W : 동촤 -> 0 : 방향 , 1 : 위치
    VertexOutput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    //  NDC 공간 - FVF_RHWXYZ
    //output.Position = input.Position;

    output.Uv = input.Uv;
    return output; // 반환값이 픽셀의 위치
}

// --------------------------------------------------------------------- //
//  Pixel Shader
// --------------------------------------------------------------------- //
const float pi = 3.14159265359;
const float triangleScale = 0.816497161855865; // ratio of edge length and height
const float3 orange = float3(0.937, 0.435, 0.0);
float Time;
float2 Resolution;

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float4 getTriangleCoords(float2 uv)
{
    uv.y /= triangleScale;
    uv.x -= uv.y / 2.0;
    float2 center = floor(uv);
    float2 local = frac(uv);
    
    center.x += center.y / 2.0;
    center.y *= triangleScale;
    
    if (local.x + local.y > 1.0)
    {
        local.x -= 1.0 - local.y;
        local.y = 1.0 - local.y;
        center.y += 0.586;
        center.x += 1.0;
    }
    else
    {
        center.y += 0.287;
        center.x += 0.5;
    }
    
    return float4(center, local);
}

float4 getLoader(float4 t)
{
    if (length(t.xy) > 1.6)
    {
        return float4(0, 0, 0, 0);
    }
    
    float angle = atan(t.y / t.x);
    float seed = rand(t.xy);
    float dst = min(t.z, min(t.w, 1.0 - t.z - t.w)) * 15.0;
    float glow = dst < pi ? pow(sin(dst), 1.5) : 0.0;
    
    return float4(lerp(orange, float3(1, 1, 1), glow * 0.07), pow(0.5 + 0.5 * sin(angle - Time * 6.0 + seed), 2.0));
}

float getBackground(float4 t) 
{
    float dst = min(t.z, min(t.w, 1.0 - t.z - t.w)) - 0.05;
	
    if (t.y > 1.9 || t.y < -2.4 || dst < 0.0) 
    {
        return 0.0;
    }

    float value = pow(0.5 + 0.5 * cos(-abs(t.x) * 0.4 + rand(t.xy) * 2.0 + Time * 4.0), 2.0) * 0.08;    
    return value * (dst > 0.05 ? 0.65 : 1.0);
}

float3 getColor(float2 uv)
{
    uv *= 2.0f / Resolution.y;
    
    float3 background = getBackground(getTriangleCoords(uv * 6.0 - float2(0.5, 0.3)));
    float4 loader = getLoader(getTriangleCoords(uv * 11.0));
    
    float3 color = lerp(background, loader.rgb, loader.a);
    return color;
}

float4 LoadingImg(VertexOutput input)
{
    float2 uv = 0;
    uv.x = (input.Uv.x * Resolution.x) - 0.5 * Resolution.x;
    uv.y = (input.Uv.y * Resolution.y) - 0.5 * Resolution.y;
    float4 color = 1;
    color.rgb = 0.25 * (getColor(uv)
                            + getColor(uv + float2(0.5, 0.0))
                            + getColor(uv + float2(0.5, 0.5))
                            + getColor(uv + float2(0.0, 0.5)));

    return color;
}

float3x3 inverse(float3x3 m)
{
    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    float b01 = a22 * a11 - a12 * a21;
    float b11 = -a22 * a10 + a12 * a20;
    float b21 = a21 * a10 - a11 * a20;

    float det = a00 * b01 + a01 * b11 + a02 * b21;

    return float3x3(b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),
                b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),
                b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) / det;
}

// --------[ Original ShaderToy begins here ]---------- //
float N21(float2 p)
{
    p = frac(p * float2(233.34, 851.73));
    p += dot(p, p + 23.45);
    return frac(p.x * p.y);
}

float inRect(float2 pos, float2 topLeft, float2 rightBottom)
{
    return step(topLeft.x, pos.x) * step(rightBottom.y, pos.y) * step(-rightBottom.x, -pos.x) * step(-topLeft.y, -pos.y);
}

float inBetween(float x, float a, float b)
{
    return step(a, x) * step(-b, -x);
}

float boxLayer(float depth, float2 uv, float size, float pos)
{

    const float fullDepth = 4.0;
    
    float2 boxCenter = float2(fullDepth * pos, sin(Time * 10.0 * (0.3 + 0.7 * N21(float2(depth, size)))));
    float boxHalfSize = size * 0.5;
    
    float m = 0.0;

    m = inRect(uv, boxCenter + float2(-boxHalfSize, boxHalfSize), boxCenter + float2(boxHalfSize, -boxHalfSize))
    * inRect(uv, float2(0.0, 1.0), float2(3.99, -1.0));
    return clamp(m, 0.0, 1.0);
}

float UVx;
float4 LoadingBar(VertexOutput input)
{
    float2 uv = input.Uv;

    // Time varying pixel color
    // vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));
    float sWidth = Resolution.x / Resolution.y;
    const float barWidthRatio = 1.0;
    float barHeight = 0.03;
    uv.x = uv.x * sWidth;
    float3x3 m_s2bar = float3x3(float3(barWidthRatio * sWidth, 0.0, 0.0),
                        float3(0.0, barWidthRatio * sWidth, 0.0),
                        float3((1.0 - barWidthRatio) * sWidth * 0.5, 1.0 - barHeight, 1.0));
    float2 uv_bar = mul(float3(uv.x, uv.y, 1), inverse(m_s2bar)).xy;
    float isInBaseRect = inRect(uv_bar, float2(0.0, 0.5 * barHeight), float2(1.0, -0.5 * barHeight));
    float isInActiveRect = inRect(uv_bar, float2(0.0, 0.5 * barHeight), float2(UVx, -0.5 * barHeight));
    float3 baseColor = float3(0.12941, 0.13725, 0.17647);
    // vec3 activeColor = vec3(0.0, uv_bar.x, 0.0);
    float3 activeColor = lerp(float3(0.937, 0.435, 0.0), float3(0.96078, 0.23529, 0.43529), uv_bar.x);
    float3 color = float3(0.0, 0.0, 0.0);
    color = lerp(color, baseColor, isInBaseRect);
    color = lerp(color, activeColor, isInActiveRect);
    
    float3x3 T_bar2top = float3x3(
    	float3(0.5 * barHeight, 0.0, 0.0),
        float3(0.0, 0.5 * barHeight, 0.0),
		float3(UVx, 0.0, 1.0)
    );

    float2 topCord = mul(float3(uv_bar, 1.0), inverse(T_bar2top)).xy;
    
    float sizes[10];

    sizes[0] = 0.64443028954883467;
    sizes[1] = 0.5305055282034009;
    sizes[2] = 0.663223756594665;
    sizes[3] = 0.7904855321774765;
    sizes[4] = 0.58575556655444496;
    sizes[5] = 0.4690261013697286;
    sizes[6] = 0.40226518516562614;
    sizes[7] = 0.935630139708542;
    // sizes[8] =  0.30465976518251916;
    // sizes[9] =  0.6511662264743197;

    float inBoxes = 0.0;
    float depthStep = 1.0 / 10.0;
    // for (float j = 0.0; j < 10.0; j++) {
    // 	// float depth = fract(i + iTime * 0.6);
    // 	float depth = j * depthStep;
    //     inBoxes += boxLayer(depth, topCord, sizes[int(j)]);
    // }

    for (float j = 0.0; j < 1.0; j += 0.125)
    {
    	// float depth = fract(i + iTime * 0.6);
        float depth = j;
        float pos = frac(j + Time * 0.6);
        inBoxes += boxLayer(depth, topCord, sizes[int(j * 8.0)], pos);
    }
    color = lerp(color, activeColor, clamp(inBoxes, 0.0, 1.0) * inBetween(uv_bar.x, 0.0, 1.0));

    // Output to screen
    return float4(color, 1.0);
}

SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Map;
Texture2D MapSecond;

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 color = 1;
    color = LoadingImg(input);
    color += LoadingBar(input);
    
    return color;
}

// --------------------------------------------------------------------- //
//  States
// --------------------------------------------------------------------- //
DepthStencilState Depth
{
    DepthEnable = false;
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
//  Technique
// --------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetDepthStencilState(Depth, 0);
        SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}