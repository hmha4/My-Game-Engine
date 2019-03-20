// --------------------------------------------------------------------- //
//  Compute Shader
// --------------------------------------------------------------------- //
float3 PixelPosition;

static const float PI = 3.14159265f;
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
    int width, height;
    HeightTexture.GetDimensions(width, height);

    PixelPosition.z = (PixelSize - 1) - PixelPosition.z;

    int2 resUv;

    [flatten]
    if (dispatchThreadId.x > width - 1)
        resUv.x = width - 1;
    else
        resUv.x = dispatchThreadId.x;

    [flatten]
    if (dispatchThreadId.y > height - 1)
        resUv.y = height - 1;
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

    //[flatten]
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
    if (BrushType == 2)
    {
        float dx = resUv.x - PixelPosition.x;
        float dy = resUv.y - PixelPosition.z;

        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= BrushRange * 2)
        {
            color.r += BrushPower / 255.0f;
        }
    }
    if (BrushType == 3)
    {
        float dx = resUv.x - PixelPosition.x;
        float dy = resUv.y - PixelPosition.z;

        float dist = sqrt(dx * dx + dy * dy);
        if (dist <= BrushRange * 2)
        {
            dist = ((BrushRange * 2) - dist) / (float) (BrushRange * 2) * (PI / 2.0f);

        
            color.r += (BrushPower / 255.0f) * sin(dist);
        }
    }
    if (BrushType == 4)
    {
        int count = 0;
        float average = 0.0f;

        for (int x = resUv.x - 1; x <= resUv.x + 1; x++)
        {
            for (int y = resUv.y - 1; y <= resUv.y + 1; y++)
            {
                if (x < 0 || x > width || y < 0 || y > height)
                    continue;

                count++;
                average += HeightTexture[int2(x, y)];
            }
        }

        if (count > 0)
        {
            if ((resUv.x >= (PixelPosition.x - (BrushRange * 2))) &&
            (resUv.x <= (PixelPosition.x + (BrushRange * 2))) &&
            (resUv.y >= (PixelPosition.z - (BrushRange * 2))) &&
            (resUv.y <= (PixelPosition.z + (BrushRange * 2))))
            {
                color.r = average / (float) count;
            }
        }
            
    }

    if (color.r <= 0)
        color.r = 0.0f;

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