struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

Texture2D gDiffuseMap : register(t0);

SamplerState samplerLinear : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

    // Sample texture
    float4 texColor = gDiffuseMap.Sample(samplerLinear, input.mTexCoord);

    return texColor;
}