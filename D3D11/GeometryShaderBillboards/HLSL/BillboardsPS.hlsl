#include "Lights.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirectionalLight;
	SpotLight gSpotLight;
	float3 gEyePositionW;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gViewProjection;
    Material gMaterial;
};
    
SamplerState samplerLinear : register(s0);

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
    uint mPrimitiveID : SV_PrimitiveID;
};

Texture2DArray gTreeMapArray : register(t0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

    // Compute vector from pixel position to eye.
	float3 toEyeW = gEyePositionW - input.mPositionW;

    // Cache the distance to the eye from this surface point.
	float distToEye = length(toEyeW);

	// Normalize.
	toEyeW /= distToEye;

    // Sample texture.
	float3 uvw = float3(input.mTexCoord, input.mPrimitiveID % 2);
	float4 texColor = gTreeMapArray.Sample(samplerLinear, uvw);

	// Discard pixel if texture alpha < 0.05.  Note that we do this
	// test as soon as possible so that we can potentially exit the shader 
	// early, thereby skipping the rest of the shader code.
	clip(texColor.a - 0.05f);

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

	computeSpotLight(gMaterial, gSpotLight, input.mPositionW, input.mNormalW, toEyeW, 
        ambientContribution, diffuseContribution, specularContribution);
	ambient += ambientContribution;  
	diffuse += diffuseContribution;
	specular += specularContribution;
	   
	float4 finalColor = texColor * (ambient + diffuse) + specular;

	// Common to take alpha from diffuse material and texture.
	finalColor.a = gMaterial.mDiffuse.a * texColor.a;

    return finalColor;
}