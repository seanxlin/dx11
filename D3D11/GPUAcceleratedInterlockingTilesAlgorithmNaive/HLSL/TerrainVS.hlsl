#include "Lights.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInverseTranspose;
	float4x4 gWorldViewProjection;
};

struct VSInput
{
	float3 mPositionL : POSITION;
	float3 mNormalL : NORMAL;
    float3 mTangentL : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

struct VSOutput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

VSOutput main(in const VSInput vsInput)
{
	VSOutput vsOutput;
	
	// Transform to world space.
    const float4 vertexPositionL = float4(vsInput.mPositionL, 1.0f);
	vsOutput.mPositionW = mul(vertexPositionL, gWorld).xyz;

    const float3x3 reducedWorldInverseTranspose = (float3x3)gWorldInverseTranspose; 
	vsOutput.mNormalW = mul(vsInput.mNormalL, reducedWorldInverseTranspose);

    const float3x3 reducedWorld = (float3x3)gWorld;
    vsOutput.mTangentW = mul(vsInput.mTangentL, reducedWorld);
		
	// Transform to homogeneous clip space.
	vsOutput.mPositionH = mul(vertexPositionL, gWorldViewProjection);

    // Output vertex attributes for interpolation across triangle.
	vsOutput.mTexCoord = vsInput.mTexCoord;
	
	return vsOutput;
}