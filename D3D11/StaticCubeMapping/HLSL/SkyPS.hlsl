// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap : register(t0);

SamplerState mySampler : register(s0);

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionL : POSITION;
};

float4 main(PixelShaderInput input) : SV_Target
{
	return gCubeMap.Sample(mySampler, input.mPositionL);
}