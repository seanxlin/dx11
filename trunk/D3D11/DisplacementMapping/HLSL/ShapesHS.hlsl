#define NUM_CONTROL_POINTS 3
#define MAX_TESS_FACTOR 64.0f

cbuffer cbPerFrame : register(b0)
{
    float gTessellationFactor;
};

struct PatchTess
{
	float mEdgeTess[3] : SV_TessFactor;
	float mInsideTess : SV_InsideTessFactor;
};

struct HullShaderInput
{
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

struct HullShaderOutput
{
	float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
	float3 mTangentW : TANGENT;
	float2 mTexCoord : TEXCOORD;
};

PatchTess ConstantHS(InputPatch<HullShaderInput, NUM_CONTROL_POINTS> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess patchTess;
	
	patchTess.mEdgeTess[0] = gTessellationFactor;
	patchTess.mEdgeTess[1] = gTessellationFactor;
	patchTess.mEdgeTess[2] = gTessellationFactor;
	patchTess.mInsideTess = gTessellationFactor;

	return patchTess;
}


[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(MAX_TESS_FACTOR)]
HullShaderOutput main(InputPatch<HullShaderInput, NUM_CONTROL_POINTS> patch, 
           uint outputControlPointID : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullShaderOutput output;
	output.mPositionW = patch[outputControlPointID].mPositionW;
	output.mNormalW = patch[outputControlPointID].mNormalW;
	output.mTangentW = patch[outputControlPointID].mTangentW;
	output.mTexCoord = patch[outputControlPointID].mTexCoord;
	
	return output;
}