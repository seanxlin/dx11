#include "Lights.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirectionalLight;
    Material gMaterial;
    float3 gEyePositionW;
};

SamplerState samplerLinear : register(s0);

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

Texture2D gDiffuseMap : register(t0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

    // Compute vector from pixel position to eye.
	float3 toEyeW = normalize(gEyePositionW - input.mPositionW);

    // Sample texture
    float4 texColor = gDiffuseMap.Sample(samplerLinear, input.mTexCoord);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 ambientContribution;
    float4 diffuseContribution;
    float4 specularContribution;

	computeDirectionalLight(gMaterial, gDirectionalLight, input.mNormalW, toEyeW, 
        ambientContribution, diffuseContribution, specularContribution);
	ambient += ambientContribution;  
	diffuse += diffuseContribution;
	specular += specularContribution;
	   
	float4 finalColor = texColor * (ambient + diffuse) + specular;

	// Common to take alpha from diffuse material and texture.
	finalColor.a = gMaterial.mDiffuse.a * texColor.a;

    return finalColor;
}