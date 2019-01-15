//  입력 버퍼(구조화 버퍼, 읽기 전용)
ByteAddressBuffer data;
//  출력 버퍼(구조화 버퍼, 읽기, 쓰기 가능)
RWByteAddressBuffer result;

[numthreads(128, 1, 1)]  //  스레드 그룹 스레드 수
void CS(uint3 dispatchId : SV_DispatchThreadID)
{
    int fetchAddress = dispatchId.x * 19 * 4;

    //  float4 temp     = asfloat(data.Load4(fetchAddress));
    //  float4 tempX    = data.Load(fetchAddress);
    //  float2 tempXY   = data.Load2(fetchAddress);
    //  float  tempY    = data.Load(fetchAddress + 4);

    float3 pos = asfloat(data.Load3(fetchAddress + 0));
    float2 uv = asfloat(data.Load2(fetchAddress + 12));
    float3 nor = asfloat(data.Load3(fetchAddress + 20));
    float3 tan = asfloat(data.Load3(fetchAddress + 32));

    result.Store3(fetchAddress + 0, asuint(pos));
    result.Store3(fetchAddress + 20, asuint(nor));
    result.Store3(fetchAddress + 32, asuint(tan));
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
        SetComputeShader(CompileShader(cs_5_0, CS()));
    }

}