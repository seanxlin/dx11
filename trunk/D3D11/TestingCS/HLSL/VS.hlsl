cbuffer cbPerFrame : register(b0)
{
	float4x4 gWorld;
    float4x4 gWorldInverseTranspose;
    float4x4 gViewProjection;
    float4x4 gTextureScale;
};

struct VSInput
{
	float3 mPositionL : POSITION;
    float3 mNormalL : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

VSOutput main(in const VSInput vsInput)
{
	VSOutput vsOutput;
	
    const float4 vertexPositionL = float4(vsInput.mPositionL, 1.0f);
    const float4 vertexPositionW = mul(vertexPositionL, gWorld);
    vsOutput.mPositionW = vertexPositionW.xyz;

    vsOutput.mPositionH = mul(vertexPositionW, gViewProjection);
        
    const float4 normalL = float4(vsInput.mNormalL, 1.0f);
    vsOutput.mNormalW = mul(normalL, gWorldInverseTranspose);

    const float4 texCoord = float4(vsInput.mTexCoord, 0.0f, 1.0f);
	vsOutput.mTexCoord = mul(texCoord, gTextureScale).xy;
	
	return vsOutput;
}