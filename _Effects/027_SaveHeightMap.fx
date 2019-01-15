
struct Data
{
    float data;
};

Texture2D<float> Input;
RWStructuredBuffer<Data> Output;

[numthreads(32, 32, 1)] // data size보다 커야함
void CS(int3 groupThreadId : SV_GroupThreadId, int3 dispatchThreadId : SV_DispatchThreadId)
{
    int width, height;
    Input.GetDimensions(width, height);

    if (dispatchThreadId.x <= width && dispatchThreadId.y <= height)
        Output[width * dispatchThreadId.y + dispatchThreadId.x].data = Input[dispatchThreadId.xy];
}


//-----------------------------------------------------------------------------
// Techinque
//-----------------------------------------------------------------------------

technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CS()));
    }
}