#include "000_Header.fx"

// Renderstates 
DepthStencilState DisableDepth
{
    DepthEnable = false;
    DepthWriteMask = 0;
};
DepthStencilState DisableDepthWrite
{
    DepthEnable = true;
    DepthWriteMask = 0;
};

DepthStencilState EnableDepth
{
    DepthEnable = true;
    DepthWriteMask = All;
    
};

RasterizerState NoCull
{
    CullMode = None;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = false;
    BlendEnable[0] = true;
    SrcBlend = One;
    DestBlend = One;
    BlendOp = Add;
    SrcBlendAlpha = Zero;
    DestBlendAlpha = Zero;
    BlendOpAlpha = Add;
    RenderTargetWriteMask[0] = 0x0F;
};
BlendState MaximumBlending
{
    AlphaToCoverageEnable = false;
    BlendEnable[0] = true;
    SrcBlend = One;
    DestBlend = One;
    BlendOp = Max;
    SrcBlendAlpha = Zero;
    DestBlendAlpha = Zero;
    BlendOpAlpha = Add;
    RenderTargetWriteMask[0] = 0x0F;
};
BlendState NoBlending
{
    BlendEnable[0] = false;
};

SamplerState PointSample
{
    Filter = Min_Mag_Mip_Point;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState LinearSample
{
    Filter = Min_Mag_Mip_Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};

static const float e = 2.71828183f;
static const float pi = 3.14159265f;

cbuffer LightningAppearance
{
    float3 ColorInside; // color of the inside of the beam
    float ColorFallOffExponent; // determines how quickly the color changes from
										// inside to outside
	
    float3 ColorOutside; // color of the outside of the beam
    float2 BoltWidth; // size in world space of the beam
	

};

cbuffer LightningStructure
{
	// for ZigZag pattern
    float2 ZigZagFraction; // determines at which percentage the segment will be broken
    float2 ZigZagDeviationRight; // min and max of deviation in segments local frame
    float2 ZigZagDeviationUp; // min and max of deviation in segments local frame
    float ZigZagDeviationDecay; // controls how fast the deviation get smaller
	
	// for Fork pattern
    float2 ForkFraction; // similiar above, but for fork pattern
    float2 ForkZigZagDeviationRight;
    float2 ForkZigZagDeviationUp;
    float ForkZigZagDeviationDecay;
	
    float2 ForkDeviationRight;
    float2 ForkDeviationUp;
    float2 ForkDeviationForward;
    float ForkDeviationDecay;

    float2 ForkLength; // min and max of length of fork segments, in world space
    float ForkLengthDecay; // decay of length
};

#define MaxTargets 8
cbuffer LightningChain
{
    float3 ChainSource;

    float4 ChainTargetPositions[MaxTargets];
	
    int NumTargets;
};

bool Fork;
uint SubdivisionLevel;
float AnimationSpeed;
float Charge;

// decay based on global subdivision level
float Decay(float amount)
{
    return exp(-amount * SubdivisionLevel);
}

// decay based in explicit level
float Decay(float2 amount, uint level)
{
    return amount.x * exp(-amount.y * level);
}

// Random number generation
// found in numerical recipes
// http://www.library.cornell.edu/nr/bookcpdf/c7-1.pdf

// per shader global variable to keep track of the last random number 
int random_x;

#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0f/float(RANDOM_IM))
#define RANDOM_IQ 127773
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

float Random()
{
    int k;
    float ans;
	
    random_x ^= RANDOM_MASK; //XORing with MASK allows use of zero and other
    k = random_x / RANDOM_IQ; //simple bit patterns for idum.
    random_x = RANDOM_IA * (random_x - k * RANDOM_IQ) - RANDOM_IR * k; //Compute idum=(IA*idum) % IM without overif
    if (random_x < 0) 
        random_x += RANDOM_IM; //flows by Schrage’s method.
	
    ans = RANDOM_AM * random_x; //Convert idum to a floating result.
    random_x ^= RANDOM_MASK; //Unmask before return.
	
    return ans;
}

void RandomSeed(int value)
{
    random_x = value;
    Random();
}

float Random(float low, float high)
{
    float v = Random();
    return low * (1.0f - v) + high * v;
}

float3 Random(float3 low, float3 high)
{
    float3 v = float3(Random(), Random(), Random());
    return low * (1.0f - v) + high * v;
}

// little debug helpers
float3 colors[] =
{
    float3(1.0f, 0.0f, 0.0f),
	float3(0.0f, 1.0f, 0.0f),
	float3(0.0f, 0.0f, 1.0f),
	float3(0.0f, 1.0f, 1.0f),
	float3(1.0f, 0.0f, 1.0f),
	float3(1.0f, 1.0f, 0.0f),
	float3(0.0f, 0.0f, 0.0f),
	float3(1.0f, 1.0f, 1.0f),

	0.5f * float3(1.0f, 0.0f, 0.0f),
	0.5f * float3(0.0f, 1.0f, 0.0f),
	0.5f * float3(0.0f, 0.0f, 1.0f),
	0.5f * float3(0.0f, 1.0f, 1.0f),
	0.5f * float3(1.0f, 0.0f, 1.0f),
	0.5f * float3(1.0f, 1.0f, 0.0f),
	0.5f * float3(1.0f, 1.0f, 1.0f),
};

// Subdivision
struct SubdivideVertex
{
    float3 Start : Start; // start of segment
    float3 End : End; // end of segment
    float3 Up : Up; // up vector, specifying frame of orientation for deviation parameters
    uint Level : Level; // n + 1 for forked segment, n for jittered segments
};

SubdivideVertex SubdivideVS(in SubdivideVertex input)
{
    SubdivideVertex output = input;

    return output;
}

// helper data structure for passing stuff around
struct Segment
{
    float3 Start;
    float3 End;

    float3 Center;
	
    float3 Right;
    float3 Up;
    float3 Forward;
	
    uint Level;
    uint Primitive;
};


// make coordinate frame
float3 GetUp(float3 start, float3 end, float3 right)
{
    float3 forward = normalize(end - start);
    return cross(right, forward);
}

// appends vertex representing a segment to stream
void DrawLineRight
(
	float3 start,
	float3 end,
	float3 right,
	uint level,
	inout PointStream<SubdivideVertex> output)
{
    SubdivideVertex v = { start, end, GetUp(start, end, right), level };
    output.Append(v);
}

// subdivision by splitting segment into two and randomly moving split point
void PatternZigZag(in Segment segment, inout PointStream<SubdivideVertex> output)
{
    float2 delta = Decay(ZigZagDeviationDecay) * float2(Random(ZigZagDeviationRight.x, ZigZagDeviationRight.y), Random(ZigZagDeviationUp.x, ZigZagDeviationUp.y));
	
    float3 jittered = lerp(segment.Start, segment.End, Random(ZigZagFraction.x, ZigZagFraction.y)) +
			delta.x * segment.Right + delta.y * segment.Up;
	
    DrawLineRight(segment.Start, jittered, segment.Right, segment.Level, output);
    DrawLineRight(jittered, segment.End, segment.Right, segment.Level, output);

}

// subdivision by splitting segment into two and randomly moving split point
// and adding a branch segment between the split position and the random end point
void PatternFork(in Segment segment, inout PointStream<SubdivideVertex> output)
{
    float2 delta = Decay(ForkZigZagDeviationDecay) * float2(Random(ForkZigZagDeviationRight.x, ForkZigZagDeviationRight.y), Random(ForkZigZagDeviationUp.x, ForkZigZagDeviationUp.y));
	
    float3 jittered = lerp(segment.Start, segment.End, Random(ForkFraction.x, ForkFraction.y)) +
			delta.x * segment.Right + delta.y * segment.Up;
	
    DrawLineRight(segment.Start, jittered, segment.Right, segment.Level, output);
    DrawLineRight(jittered, segment.End, segment.Right, segment.Level, output);
	

    float3 fork_dir = normalize(segment.Right);

	
    float3 f_delta = Decay(ForkDeviationDecay) * float3(Random(ForkDeviationRight.x, ForkDeviationRight.y), Random(ForkDeviationUp.x, ForkDeviationUp.y), Random(ForkDeviationForward.x, ForkDeviationForward.y));
    float f_length = Random(ForkLength.x, ForkLength.y) * Decay(ForkLengthDecay);
    float3 f_jittered = jittered + f_length * normalize(f_delta.x * segment.Right + f_delta.y * segment.Up + f_delta.z * segment.Forward);
	
    DrawLineRight(jittered, f_jittered, segment.Forward, segment.Level + 1, output);
	
}

// decides whether to fork or to jitter bases upon uniform parameter
[MaxVertexCount(3)]
void SubdivideGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout PointStream<SubdivideVertex> output
)
{
    RandomSeed(primitive_id + 1 + Time * AnimationSpeed);

    float3 center = 0.5f * (input[0].Start + input[0].End);
	
    Segment segment;
	
    segment.Start = input[0].Start;
    segment.End = input[0].End;
	
    segment.Center = 0.5f * (segment.Start + segment.End);
	
    segment.Up = input[0].Up;
	
    segment.Forward = normalize(segment.End - segment.Start);
    segment.Right = normalize(cross(segment.Forward, segment.Up));
    segment.Up = normalize(cross(segment.Right, segment.Forward));
	

    segment.Level = input[0].Level;
	
    segment.Primitive = primitive_id;

    if (Fork)
        PatternFork(segment, output);
    else
        PatternZigZag(segment, output);

}

GeometryShader gs_subdivide = ConstructGSWithSO
(
	CompileShader
	(
		gs_5_0,
		SubdivideGS() 
	),
	"Start.xyz; End.xyz; Up.xyz; Level.x" 
);

technique11 Subdivide
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, SubdivideVS()));
        SetGeometryShader(gs_subdivide);
        SetPixelShader(0);
        
        SetDepthStencilState(DisableDepth, 0);
    }
}

// as there are no vertex buffers bound, we use the vertex ID to index into the constant buffer
// to connect the segments
SubdivideVertex ChainLightningVS(uint VertexId : SV_VertexID)
{
    SubdivideVertex output;

    if (0 == VertexId)
    {
        output.Start = ChainSource;
        output.End = ChainTargetPositions[0].xyz;
    }
    else
    {
        output.Start = ChainTargetPositions[VertexId - 1].xyz;
        output.End = ChainTargetPositions[VertexId].xyz;
	
    }

    output.Up = float3(0, 1, 0);
    output.Level = 0;

    return output;
}

technique11 ChainLightning
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, ChainLightningVS()));
        SetGeometryShader(gs_subdivide);
        SetPixelShader(0);
        
        SetDepthStencilState(DisableDepth, 0);
    }
}


// for debugging
struct LinesOutVertexGS2PS
{
    float4 Position : SV_Position;
    uint Level : Level;
};


SubdivideVertex LinesOutVS(in SubdivideVertex input, in uint id : SV_VertexID)
{
    SubdivideVertex output;

    output = input;

    return output;
}

[MaxVertexCount(3)]
void LinesOutGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout LineStream<LinesOutVertexGS2PS> output
)
{
    LinesOutVertexGS2PS v1;
    v1.Position = mul(float4(input[0].Start, 1.0f), World);
    v1.Position = mul(v1.Position, View);
    v1.Position = mul(v1.Position, Projection);
    v1.Level = input[0].Level;
    output.Append(v1);

    LinesOutVertexGS2PS v2;
    v2.Position = mul(float4(input[0].End, 1.0f), World);
    v2.Position = mul(v1.Position, View);
    v2.Position = mul(v1.Position, Projection);
    v2.Level = input[0].Level;
    output.Append(v2);
	
    output.RestartStrip();
}

float4 LinesOutPS(in LinesOutVertexGS2PS input) : SV_Target
{
    return float4(colors[input.Level], 1.0f);
}

technique11 LinesOut
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, LinesOutVS()));
        SetGeometryShader(CompileShader(gs_5_0, LinesOutGS()));
        SetPixelShader(CompileShader(ps_5_0, LinesOutPS()));
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}


struct BoltOutVertexGS2PS
{
    float4 Position : SV_Position;
    float2 Gradient : Gradient;
    int Level : Level;
    float4 oPosition : Position1;
};

SubdivideVertex BoltOutVS(in SubdivideVertex input, in uint id : SV_VertexID)
{
    SubdivideVertex output;
    output = input;

    return output;
}

// generate camera and segment aligned quads with inter segment gaps filled
[MaxVertexCount(8)]
void BoltOutGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout TriangleStream<BoltOutVertexGS2PS> output
)
{
	// vs stands for view space
    float3 vs_start = mul(float4(input[0].Start, 1.0f), World).xyz;
    //vs_start = mul(float4(vs_start, 1.0f), View).xyz;
    float3 vs_end = mul(float4(input[0].End, 1.0f), World).xyz;
    //vs_end = mul(float4(vs_end, 1.0f), View).xyz;
    float3 vs_forward = normalize(vs_end - vs_start);
	
    float width = Decay(BoltWidth, input[0].Level);
	
    float3 right = width * normalize(cross(vs_forward, float3(0, 0, 1)));
	
    float x = 1;
    float y = 1;

    static const bool close_gaps = true;


    if (close_gaps)
    {
        BoltOutVertexGS2PS v0;
        v0.oPosition = float4(vs_start - right - width * vs_forward, 1.0f);
        v0.Position = mul(float4(vs_start - right - width * vs_forward, 1.0f), View);
        v0.Position = mul(v0.Position, Projection);
        v0.Gradient = float2(-x, 1);
        v0.Level = input[0].Level;
        output.Append(v0);
	
        BoltOutVertexGS2PS v1; 
        v1.oPosition = float4(vs_start + right - width * vs_forward, 1.0f);
        v1.Position = mul(float4(vs_start + right - width * vs_forward, 1.0f), View);
        v1.Position = mul(v1.Position, Projection);
        v1.Gradient = float2(x, 1);
        v1.Level = input[0].Level;
        output.Append(v1);
	
    }

    BoltOutVertexGS2PS v2;
    v2.oPosition = float4(vs_start - right, 1.0f);
    v2.Position = mul(float4(vs_start - right, 1.0f), View);
    v2.Position = mul(v2.Position, Projection);
    v2.Gradient = float2(-x, 0);
    v2.Level = input[0].Level;
    output.Append(v2);
	
    BoltOutVertexGS2PS v3;
    v3.oPosition = float4(vs_start + right, 1.0f);
    v3.Position = mul(float4(vs_start + right, 1.0f), View);
    v3.Position = mul(v3.Position, Projection);
    v3.Gradient = float2(x, 0);
    v3.Level = input[0].Level;
    output.Append(v3);

    BoltOutVertexGS2PS v4;
    v4.oPosition = float4(vs_end - right, 1.0f);
    v4.Position = mul(float4(vs_end - right, 1.0f), View);
    v4.Position = mul(v4.Position, Projection);
    v4.Gradient = float2(-x, 0);
    v4.Level = input[0].Level;
    output.Append(v4);
	
    BoltOutVertexGS2PS v5;
    v5.oPosition = float4(vs_end + right, 1.0f);
    v5.Position = mul(float4(vs_end + right, 1.0f), View);
    v5.Position = mul(v5.Position, Projection);
    v5.Gradient = float2(x, 0);
    v5.Level = input[0].Level;
    output.Append(v5);
	

    if (close_gaps)
    {
        BoltOutVertexGS2PS v6;
        v6.oPosition = float4(vs_end - right + width * vs_forward, 1.0f);
        v6.Position = mul(float4(vs_end - right + width * vs_forward, 1.0f), View);
        v6.Position = mul(v6.Position, Projection);
        v6.Gradient = float2(-x, 1);
        v6.Level = input[0].Level;
        output.Append(v6);
	
        BoltOutVertexGS2PS v7;
        v7.oPosition = float4(vs_end + right + width * vs_forward, 1.0f);
        v7.Position = mul(float4(vs_end + right + width * vs_forward, 1.0f), View);
        v7.Position = mul(v7.Position, Projection);
        v7.Gradient = float2(x, 1);
        v7.Level = input[0].Level;
        output.Append(v7);
    }
	 
    output.RestartStrip();
}

Texture2D ScreenDepth;

float4 BoltOutPS(in BoltOutVertexGS2PS input) : SV_Target
{
    float4 color = 0;
    float sceneZ = ScreenDepth.Load(int3(input.Position.xy, 0));

    if (dot(sceneZ, 1.0f) != 0)
    {
        float4 clipPos = mul(input.oPosition, View);
        clipPos = mul(clipPos, Projection);

        clipPos.z /= clipPos.w;
        if (clipPos.z < sceneZ)
        {
            float f = saturate(length(input.Gradient));
            float brightness = 1 - f;
            float color_shift = saturate(pow(1 - f, ColorFallOffExponent));

            color = brightness * float4(lerp(ColorOutside, ColorInside, color_shift), 1.0f);
        }
    }
    else
    {
        float f = saturate(length(input.Gradient));
        float brightness = 1 - f;
        float color_shift = saturate(pow(1 - f, ColorFallOffExponent));

        color = brightness * float4(lerp(ColorOutside, ColorInside, color_shift), 1.0f);
    }
    

    return color;
}

technique11 BoltOut
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, BoltOutVS()));
        SetGeometryShader(CompileShader(gs_5_0, BoltOutGS()));
        SetPixelShader(CompileShader(ps_5_0, BoltOutPS()));
     
        SetDepthStencilState(DisableDepthWrite, 0);
        SetRasterizerState(NoCull);
        
        SetBlendState(AdditiveBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}


technique11 ShowLines
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, BoltOutVS()));
        SetGeometryShader(CompileShader(gs_5_0, LinesOutGS()));
        SetPixelShader(CompileShader(ps_5_0, LinesOutPS()));
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}


bool erode;
bool dilate;
bool blur;

struct PassThroughApp2VS
{
    float3 Position : Position;
};

struct PassThroughVS2PS
{
    float4 Position : SV_Position;
};

PassThroughVS2PS PassThroughVS(in PassThroughApp2VS input)
{
    PassThroughVS2PS output;
    output.Position = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    return output;
}

float4 PassThroughPS(in PassThroughVS2PS input) : SV_Target
{
    return 0.5f * float4(1.0f, 1.0f, 1.0f, 1.0f);
}

technique11 PassThrough
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, PassThroughVS()));
        SetGeometryShader(0);
        SetPixelShader(CompileShader(ps_5_0, PassThroughPS()));
        
        SetRasterizerState(NoCull);
        SetDepthStencilState(DisableDepthWrite, 0);
    }
}



float4 QuadPositionTexCoords[] =
{
    float4(-1, -1, 0, 1),
	float4(-1, +1, 0, 0),
	float4(+1, -1, 1, 1),
	float4(+1, +1, 1, 0),
};


struct QuadVS2PS
{
    float4 Position : SV_Position;
    float2 TexCoord : TexCoord;
};

QuadVS2PS QuadVS(uint VertexId : SV_VertexID)
{
    QuadVS2PS output;
    output.Position = float4(QuadPositionTexCoords[VertexId].xy, 0, 1);
    output.TexCoord = QuadPositionTexCoords[VertexId].zw;

    return output;
}
 
Texture2D<float4> buffer;
Texture2D gradient;
float2 BufferTexelSize;


float4 AddBufferPS(in QuadVS2PS input) : SV_Target
{
    return buffer.SampleLevel(LinearSample, input.TexCoord, 0);
}

float3 BlurSigma;
static const int blur_search_width = 8;

static const int blur_search_start = -blur_search_width;
static const int blur_search_end = blur_search_width + 1;
static const float blur_scale = 2.0f;


float Gaussian(float2 xy, float sigma)
{
    return exp(-(dot(xy, xy) / (2.0f * sigma * sigma))) / (2.0f * pi * sigma * sigma);
}

float3 Gaussian(float2 xy, float3 sigma)
{
    float3 sigma_prime = sigma * sigma * 2;
    float3 d = dot(xy, xy);
	 
    return exp(-d / sigma_prime) / (pi * sigma_prime);

}
float3 Gaussian(float d, float3 sigma)
{
    float3 sigma_prime = sigma * sigma * 2;
    return exp(-abs(d) / sigma_prime) / (pi * sigma_prime);

}

float4 BlurBufferPS(in QuadVS2PS input, uniform bool horizontal) : SV_Target
{
    float4 sum = float4(0, 0, 0, 0);
	

    if (horizontal)
    {
	
        for (int i = blur_search_start; i < blur_search_end; ++i)
            sum.rgb += Gaussian(i, BlurSigma) * buffer.SampleLevel(LinearSample, input.TexCoord + BufferTexelSize * float2(0.5f + 2.0f * i, 0.5f), 0).rgb;
    }
    else
    {
        for (int i = blur_search_start; i < blur_search_end; ++i)
            sum.rgb += Gaussian(i, BlurSigma) * buffer.SampleLevel(LinearSample, input.TexCoord + BufferTexelSize * float2(0.5f, 0.5f + 2 * i), 0).rgb;

    }

    return blur_scale * sum;
}


technique11 AddBuffer
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, QuadVS()));
        SetGeometryShader(0);
        SetPixelShader(CompileShader(ps_5_0, AddBufferPS()));
        
        SetRasterizerState(NoCull);
        SetBlendState(AdditiveBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
        SetDepthStencilState(DisableDepth, 0);
    }
}

technique11 BlurBufferHorizontal
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, QuadVS()));
        SetGeometryShader(0);
        SetPixelShader(CompileShader(ps_5_0, BlurBufferPS(true)));
        
        SetRasterizerState(NoCull);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
        SetDepthStencilState(DisableDepth, 0);
    }
}
technique11 BlurBufferVertical
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, QuadVS()));
        SetGeometryShader(0);
        SetPixelShader(CompileShader(ps_5_0, BlurBufferPS(false)));
        
        SetRasterizerState(NoCull);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
        SetDepthStencilState(DisableDepth, 0);
    }
}
float4 DownSample2x2PS(in QuadVS2PS input) : SV_Target
{
    return buffer.SampleLevel(LinearSample, input.TexCoord, 0);
}

technique11 DownSample2x2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, QuadVS()));
        SetGeometryShader(0);
        SetPixelShader(CompileShader(ps_5_0, DownSample2x2PS()));
        
        SetRasterizerState(NoCull);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
        SetDepthStencilState(DisableDepth, 0);
    }
}
