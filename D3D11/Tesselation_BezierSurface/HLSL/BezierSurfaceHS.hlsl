#define NUM_CONTROL_POINTS 16
#define MAX_TESS_FACTOR 64.0f

cbuffer cbPerFrame : register(b0)
{
    float gTessellationFactor;
};

struct PatchTess
{
	float mEdgeTess[4] : SV_TessFactor;
	float mInsideTess[2] : SV_InsideTessFactor;
};

struct HullShaderInput
{
    float3 mPositionL : POSITION;
};

struct HullShaderOutput
{
    float3 mPositionL : POSITION;
};

PatchTess ConstantHS(InputPatch<HullShaderInput, NUM_CONTROL_POINTS> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess patchTess;
	
	patchTess.mEdgeTess[0] = gTessellationFactor;
	patchTess.mEdgeTess[1] = gTessellationFactor;
	patchTess.mEdgeTess[2] = gTessellationFactor;
	patchTess.mEdgeTess[3] = gTessellationFactor;
	
	patchTess.mInsideTess[0] = gTessellationFactor;
	patchTess.mInsideTess[1] = gTessellationFactor;

	return patchTess;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(MAX_TESS_FACTOR)]
HullShaderOutput main(InputPatch<HullShaderInput, NUM_CONTROL_POINTS> patch, 
           uint outputControlPointID : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullShaderOutput output;
	output.mPositionL = patch[outputControlPointID].mPositionL;
	
	return output;
}