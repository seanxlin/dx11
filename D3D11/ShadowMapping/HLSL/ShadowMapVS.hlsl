cbuffer cbPerObject : register (b0)
{
	float4x4 gLightViewProjection;
	float4x4 gTexTransform;
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
	float4 mPositionH : SV_POSITION;
	float2 mTexCoord : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

    float3 positionW = mul(float4(input.mPositionL, 1.0f), input.mWorld),xyz;
	output.mPositionH = mul(float4(positionW, 1.0f), gLightViewProjection);
	output.mTexCoord = mul(float4(input.mTexCoord, 0.0f, 1.0f), gTexTransform).xy;

	return output;
}