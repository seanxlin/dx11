#include "Buffers.hlsli"

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

cbuffer cbImmutable : register(b2)
{
	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

Texture2D gDiffuseMap : register(t0);

#ifndef ALPHA_CLIP 
#define ALPHA_CLIP 0 
#endif 

#ifndef TEXTURES_ENABLED 
#define TEXTURES_ENABLED 0
#endif

#ifndef FOG_ENABLED 
#define FOG_ENABLED 0 
#endif

float4 main(in PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

	// The toEye vector is used in lighting.
	float3 toEyeW = gEyePositionW - input.mPositionW; 
	 
	// Cache the distance to the eye from this surface point.
	const float distToEye = length(toEyeW);

	// Normalize.
	toEyeW /= distToEye;

    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
#if TEXTURES_ENABLED
	// Sample texture.
	texColor = gDiffuseMap.Sample(samplerLinear, input.mTexCoord);

#if ALPHA_CLIP
	// Discard pixel if texture alpha < 0.1.  Note that we do this
	// test as soon as possible so that we can potentially exit the shader 
	// early, thereby skipping the rest of the shader code.
	clip(texColor.a - 0.1f);  
#endif
#endif

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

    float4 finalColor = texColor * (ambient + diffuse) + specular;

    //
	// Fogging
	//
#if FOG_ENABLED
	const float fogLerp = saturate((distToEye - gFogStart) / gFogRange); 

	// Blend the fog color and the lit color.
	finalColor = lerp(finalColor, gFogColor, fogLerp);	
#endif	   

	// Common to take alpha from diffuse material and texture.
	finalColor.a = gMaterial.mDiffuse.a * texColor.a;
    return finalColor;
}