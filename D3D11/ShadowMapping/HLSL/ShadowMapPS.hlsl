struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
	float2 mTexCoord : TEXCOORD;
};

Texture2D gDiffuseMap : register (t0);

SamplerState gTextureSampler : register(s0);

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.
void main(PixelShaderInput input)
{
	float4 diffuse = gDiffuseMap.Sample(gTextureSampler, input.mTexCoord);

	// Don't write transparent pixels to the shadow map.
	clip(diffuse.a - 0.15f);
}