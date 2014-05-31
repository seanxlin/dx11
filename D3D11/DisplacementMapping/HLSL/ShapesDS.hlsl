#define NUM_CONTROL_POINTS 3

cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewProjection;
    float3 gEyePositionW;
};

struct PatchTess
{
	float mEdgeTess[3] : SV_TessFactor;
	float mInsideTess : SV_InsideTessFactor;
};

struct DomainShaderInput
{
	float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
	float3 mTangentW : TANGENT;
	float2 mTexCoord : TEXCOORD;
};

struct DomainShaderOutput
{
	float4 mPositionH : SV_POSITION;
	float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
	float3 mTangentW : TANGENT;
	float2 mTexCoord : TEXCOORD;
};

SamplerState mySampler : register(s0);

Texture2D gNormalMap : register(t0);

[domain("tri")]
DomainShaderOutput main(in PatchTess patchTess, 
             in const float3 barycentric : SV_DomainLocation, 
             const OutputPatch<DomainShaderInput, NUM_CONTROL_POINTS> trianglePatch)
{
	DomainShaderOutput output;

	// Interpolate patch attributes to generated vertices.
	output.mPositionW = barycentric.x * trianglePatch[0].mPositionW + barycentric.y * trianglePatch[1].mPositionW + barycentric.z * trianglePatch[2].mPositionW;
	output.mNormalW = barycentric.x * trianglePatch[0].mNormalW  + barycentric.y * trianglePatch[1].mNormalW  + barycentric.z * trianglePatch[2].mNormalW;
	output.mTangentW = barycentric.x * trianglePatch[0].mTangentW + barycentric.y * trianglePatch[1].mTangentW + barycentric.z * trianglePatch[2].mTangentW;
	output.mTexCoord = barycentric.x * trianglePatch[0].mTexCoord + barycentric.y * trianglePatch[1].mTexCoord + barycentric.z * trianglePatch[2].mTexCoord;
	
	// Interpolating normal can unnormalize it, so normalize it.
	output.mNormalW = normalize(output.mNormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the most detailed mipmap level
	const float mipLevel = 1.0f;
	
	// Sample height map (stored in alpha channel).
	const float height = gNormalMap.SampleLevel(mySampler, output.mTexCoord, mipLevel).a;
	
	// Offset vertex along normal.
    const float scaleOffset = 1.0f;
	output.mPositionW += height * scaleOffset * output.mNormalW;
	
	// Project to homogeneous clip space.
	output.mPositionH = mul(float4(output.mPositionW, 1.0f), gViewProjection);

	return output;
}