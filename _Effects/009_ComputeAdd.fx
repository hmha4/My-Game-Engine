struct Data
{
    float3 V1;
    float2 V2;
};

//  입력 버퍼(구조화 버퍼, 읽기 전용)
StructuredBuffer<Data> Input1;
StructuredBuffer<Data> Input2;
//  출력 버퍼(구조화 버퍼, 읽기, 쓰기 가능)
RWStructuredBuffer<Data> Output;

[numthreads(64, 1, 1)]  //  스레드 그룹 스레드 수
void CS(int3 id : SV_GroupThreadId)
{
    Output[id.x].V1 = Input1[id.x].V1 + Input2[id.x].V1;
    Output[id.x].V2 = Input1[id.x].V2 + Input2[id.x].V2;
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