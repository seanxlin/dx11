#include "Lights.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gViewProjection;
    float4x4 gTexTransform;
    float4x4 gShadowTransform;
};

struct VertexShaderInput
{
    row_major float4x4 mWorld : WORLD;
	float3 mPositionL : POSITION;
	float3 mNormalL : NORMAL;
    float3 mTangentL : TANGENT;
    float2 mTexCoord : TEXCOORD;
    uint mInstanceID : SV_InstanceID;
};

struct VertexShaderOutput
{
    float4 mShadowPositionH : TEXCOORD1;
    float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	// Transform to world space space.
	output.mPositionW = mul(float4(input.mPositionL, 1.0f), input.mWorld).xyz;
	output.mNormalW = mul(input.mNormalL, (float3x3)input.mWorld);
    output.mTangentW = mul(input.mTangentL, (float3x3)input.mWorld);
		
	// Transform to homogeneous clip space.
	output.mPositionH = mul(float4(output.mPositionW, 1.0f), gViewProjection);

    // Output vertex attributes for interpolation across triangle.
	output.mTexCoord = mul(float4(input.mTexCoord, 0.0f, 1.0f), gTexTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    output.mShadowPositionH = mul(float4(input.mPositionL, 1.0f), gShadowTransform);
	
	return output;
}