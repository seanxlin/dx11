cbuffer cbPerFrame
{
	float4x4 gWorldViewProjection;
};

struct VertexShaderInput
{
	float3 mPositionL : POSITION;
};

struct VertexShaderOutput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionL : POSITION;
};
 
VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	output.mPositionH = mul(float4(input.mPositionL, 1.0f), gWorldViewProjection).xyww;
	
	// Use local vertex position as cubemap lookup vector.
	output.mPositionL = input.mPositionL;
	
	return output;
}