struct Bone
{
    float4x4 BindPose;
};

StructuredBuffer<Bone> BoneBuffer;
int BoneIndex;

ByteAddressBuffer Vertex;
RWByteAddressBuffer Result;

struct VertexTextureNormalTangentBlend
{
    float3 Position;
    float2 Uv;
    float3 Normal;
    float3 Tangent;
    float4 BlendIndices;
    float4 BlendWeights;
};

void Skinning(inout VertexTextureNormalTangentBlend vertex)
{
    float4x4 transforms = 0;
    
    [loop]
    for (int i = 0; i < 4; i++)
        transforms += mul(vertex.BlendWeights[i], BoneBuffer[(uint) vertex.BlendIndices[i]].BindPose);

    vertex.Position = mul(float4(vertex.Position, 1), transforms).xyz;
    vertex.Normal = normalize(mul(vertex.Normal, (float3x3) transforms));
    vertex.Tangent = normalize(mul(vertex.Tangent, (float3x3) transforms));
}

[numthreads(512, 1, 1)] //  스레드 그룹 스레드 수
void CS_Bone(uint3 dispatchId : SV_DispatchThreadID)
{
    int fetchAddress = dispatchId.x * 19 * 4;

    VertexTextureNormalTangentBlend vertex;
    // Vertex   Uv  Normal  Tangent BlendIndices    BlendWeight
    //  3       2   3       3       4               4
    //  0       12  20      32      44              60          80
    vertex.Position = asfloat(Vertex.Load3(fetchAddress + 0));
    vertex.Uv = asfloat(Vertex.Load2(fetchAddress + 12));
    vertex.Normal = asfloat(Vertex.Load3(fetchAddress + 20));
    vertex.Tangent = asfloat(Vertex.Load3(fetchAddress + 32));
    vertex.BlendIndices = asfloat(Vertex.Load4(fetchAddress + 44));
    vertex.BlendWeights = asfloat(Vertex.Load4(fetchAddress + 60));

    vertex.Position = mul(float4(vertex.Position, 1), BoneBuffer[BoneIndex].BindPose);
    vertex.Normal = normalize(mul(vertex.Normal, (float3x3) BoneBuffer[BoneIndex].BindPose));
    vertex.Tangent = normalize(mul(vertex.Tangent, (float3x3) BoneBuffer[BoneIndex].BindPose));

    Result.Store3(fetchAddress + 0, asuint(vertex.Position));
    Result.Store2(fetchAddress + 12, asuint(vertex.Uv));
    Result.Store3(fetchAddress + 20, asuint(vertex.Normal));
    Result.Store3(fetchAddress + 32, asuint(vertex.Tangent));
    Result.Store4(fetchAddress + 44, asuint(vertex.BlendIndices));
    Result.Store4(fetchAddress + 60, asuint(vertex.BlendWeights));
}

[numthreads(512, 1, 1)] //  스레드 그룹 스레드 수
void CS_Animation(uint3 dispatchId : SV_DispatchThreadID)
{
    int fetchAddress = dispatchId.x * 19 * 4;

    VertexTextureNormalTangentBlend vertex;
    // Vertex   Uv  Normal  Tangent BlendIndices    BlendWeight
    //  3       2   3       3       4               4
    //  0       12  20      32      44              60          80
    vertex.Position = asfloat(Vertex.Load3(fetchAddress + 0));
    vertex.Uv = asfloat(Vertex.Load2(fetchAddress + 12));
    vertex.Normal = asfloat(Vertex.Load3(fetchAddress + 20));
    vertex.Tangent = asfloat(Vertex.Load3(fetchAddress + 32));
    vertex.BlendIndices = asfloat(Vertex.Load4(fetchAddress + 44));
    vertex.BlendWeights = asfloat(Vertex.Load4(fetchAddress + 60));

    Skinning(vertex);

    Result.Store3(fetchAddress + 0, asuint(vertex.Position));
    Result.Store2(fetchAddress + 12, asuint(vertex.Uv));
    Result.Store3(fetchAddress + 20, asuint(vertex.Normal));
    Result.Store3(fetchAddress + 32, asuint(vertex.Tangent));
    Result.Store4(fetchAddress + 44, asuint(vertex.BlendIndices));
    Result.Store4(fetchAddress + 60, asuint(vertex.BlendWeights));
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
        SetComputeShader(CompileShader(cs_5_0, CS_Bone()));
    }

    pass P1
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CS_Animation()));
    }

}