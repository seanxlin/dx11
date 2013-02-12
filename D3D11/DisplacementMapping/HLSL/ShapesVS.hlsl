#include "Lights.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInverseTranspose;
    float4x4 gTexTransform;
};

struct VertexShaderInput
{
	float3 mPositionL : POSITION;
	float3 mNormalL : NORMAL;
    float3 mTangentL : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	// Transform to world space space.
	output.mPositionW = mul(float4(input.mPositionL, 1.0f), gWorld).xyz;
	output.mNormalW = mul(input.mNormalL, (float3x3)gWorldInverseTranspose);
    output.mTangentW = mul(input.mTangentL, (float3x3)gWorld);
		
    // Output vertex attributes for interpolation across triangle.
	output.mTexCoord = mul(float4(input.mTexCoord, 0.0f, 1.0f), gTexTransform).xy;
	
	return output;
}