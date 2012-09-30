#include "Buffers.hlsli"

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

    // Compute vector from pixel position to eye.
	float3 toEyeW = normalize(gEyePositionW - input.mPositionW);

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

	computePointLight(gMaterial, gPointLight, input.mPositionW, input.mNormalW, toEyeW, 
        ambientContribution, diffuseContribution, specularContribution);
	ambient += ambientContribution;  
	diffuse += diffuseContribution;
	specular += specularContribution;

	computeSpotLight(gMaterial, gSpotLight, input.mPositionW, input.mNormalW, toEyeW, 
        ambientContribution, diffuseContribution, specularContribution);
	ambient += ambientContribution;  
	diffuse += diffuseContribution;
	specular += specularContribution;
	   
	float4 finalColor = ambient + diffuse + specular;

	// Common to take alpha from diffuse material.
	finalColor.a = gMaterial.mDiffuse.a;

    return finalColor;
}