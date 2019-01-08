// --------------------------------------------------------------------- //
//  Compute Shader
// --------------------------------------------------------------------- //
float3 PixelPosition;

static const int N = 32;
static const float PixelSize = 2049;

int BrushType = 0;
int BrushRange = 10;
float BrushPower = 1;

int BrushValue = 0;

Texture2D HeightTexture;
RWTexture2D<float4> Output;

[numthreads(N, N, 1)]
void CS_Brush(int3 groupThreadId : SV_GroupThreadId, int3 dispatchThreadId : SV_DispatchThreadId)
{
 
    PixelPosition.z = (PixelSize - 1) - PixelPosition.z;

    int2 resUv;

    [flatten]
    if (dispatchThreadId.x > 2048)
        resUv.x = 2048;
    else
        resUv.x = dispatchThreadId.x;

    [flatten]
    if (dispatchThreadId.y > 2048)
        resUv.y = 2048;
    else
        resUv.y = dispatchThreadId.y;

    float4 color = HeightTexture[resUv];

    [flatten]
    if (BrushValue == 0)
    {
        BrushPower = 0;
    }
    else if (BrushValue == 1 && BrushType != 0)
    {
        BrushPower = BrushPower * 1;
    }
    else if (BrushValue == 2 && BrushType != 0)
    {
        BrushPower = BrushPower * -1;
    }

    [flatten]
    if (BrushType == 1)
    {
        if ((resUv.x >= (PixelPosition.x - (BrushRange * 2))) &&
            (resUv.x <= (PixelPosition.x + (BrushRange * 2))) &&
            (resUv.y >= (PixelPosition.z - (BrushRange * 2))) &&
            (resUv.y <= (PixelPosition.z + (BrushRange * 2))))
        {
            color.r += BrushPower / 255.0f;
        }

    }
    else if (BrushType = 2)
    {
        float dx = resUv.x - PixelPosition.x;
        float dy = resUv.y - PixelPosition.z;

        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= BrushRange * 2)
        {
            color.r += BrushPower / 255.0f;
        }
    }

    Output[resUv] = color;
}

// --------------------------------------------------------------------- //
//  Technique
// --------------------------------------------------------------------- //
technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);

        SetComputeShader(CompileShader(cs_5_0, CS_Brush()));
    }
}