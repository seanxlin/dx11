#include "Lights.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirectionalLight[3];
	float3 gEyePositionW;
};

cbuffer cbPerObject : register(b1)
{
	Material gMaterial;
};

SamplerState mySampler : register(s0);

struct PixelShaderInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    input.mNormalW = normalize(input.mNormalW); 

    // Compute vector from pixel position to eye.
	float3 toEyeW = normalize(gEyePositionW - input.mPositionW);
        
    // Normal mapping
    float3 normalMapSample = gNormalMap.Sample(mySampler, input.mTexCoord).rgb;
	float3 bumpedNormalW = normalSampleToWorldSpace(normalMapSample, input.mNormalW, input.mTangentW);

    // Sample texture
    float4 texColor = gDiffuseMap.Sample(mySampler, input.mTexCoord);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 ambientContribution;
    float4 diffuseContribution;
    float4 specularContribution;

    [unroll]
    for(int i = 0; i < 3; ++i)
    {
        computeDirectionalLight(gMaterial, gDirectionalLight[i], bumpedNormalW, toEyeW, 
            ambientContribution, diffuseContribution, specularContribution);
	    
        ambient += ambientContribution;  
	    diffuse += diffuseContribution;
	    specular += specularContribution;
    }
	   
	float4 finalColor = texColor * (ambient + diffuse) + specular;   

	// Common to take alpha from diffuse material and texture.
	finalColor.a = gMaterial.mDiffuse.a * texColor.a;

    return finalColor;
}