#include "Buffers.hlsli"

struct VertexShaderInput
{
	float3 mPositionL : POSITION;
	float3 mNormalL : NORMAL;
};

struct VertexShaderOutput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	// Transform to world space space.
	output.mPositionW = mul(float4(input.mPositionL, 1.0f), gWorld).xyz;
	output.mNormalW = mul(input.mNormalL, (float3x3)gWorldInverseTranspose);
		
	// Transform to homogeneous clip space.
	output.mPositionH = mul(float4(input.mPositionL, 1.0f), gWorldViewProjection);
	
	return output;
}