cbuffer cbPerFrame : register(b0)
{
	float4x4 gWorld;
    float4x4 gTextureScale;
};

struct VSInput
{
	float3 mPositionL : POSITION;
    float2 mTexCoord : TEXCOORD;
};

struct VSOutput
{
    float3 mPositionW : POSITION;
    float2 mTexCoord : TEXCOORD;
};

VSOutput main(in const VSInput vsInput)
{
	VSOutput vsOutput;
	
	// Transform to world space.
    const float4 vertexPositionL = float4(vsInput.mPositionL, 1.0f);
	vsOutput.mPositionW = mul(vertexPositionL, gWorld).xyz;		

    // Output vertex attributes for interpolation across triangle.
    const float4 extendedTexCoord = float4(vsInput.mTexCoord, 0.0f, 1.0f);
	vsOutput.mTexCoord = mul(extendedTexCoord, gTextureScale).xy;
	
	return vsOutput;
}