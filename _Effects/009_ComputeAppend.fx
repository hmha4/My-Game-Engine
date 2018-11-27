struct Data
{
    int Id;
    int Val;
    int Sum;
};

ConsumeStructuredBuffer<int> Input;
AppendStructuredBuffer<Data> Output;

//  x * y * z 가 768 를 넘기면 안됨
[numthreads(64, 1, 1)]
//  SV_GroupThreadId : x * y * z
//  SV_DispatchThreadId : disX * dixY * disZ * x * y * z
void CS(int3 id : SV_GroupThreadId, int3 Did : SV_DispatchThreadId)
{
    int data = Input.Consume();

    Data result;
    result.Id = id;
    result.Val = data;
    result.Sum = id + data;

    Output.Append(result);
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