#include "Lights.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirectionalLight[3];
	float3 gEyePositionW;
    Material gMaterial;
    float gTexelCellSpaceU;
    float gTexelCellSpaceV;
    float gWorldCellSpace;
};

SamplerState gHeightMapSampler : register(s0);
SamplerState gTexturesSampler : register(s1);

struct PSInput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float2 mTexCoord : TEXCOORD;
};

Texture2D gHeightMap : register(t0);
Texture2DArray gLayerMapArray : register(t1);
Texture2D gBlendMap : register(t2);

float4 main(in PSInput psInput) : SV_TARGET
{
    // Compute vector from pixel position to eye.
	const float3 toEyeW = normalize(gEyePositionW - psInput.mPositionW);
    
    // Sample layers in texture array.
	const float4 texel0 = gLayerMapArray.Sample( gTexturesSampler, float3(psInput.mTexCoord * 10.0f, 0.0f) );
	const float4 texel1 = gLayerMapArray.Sample( gTexturesSampler, float3(psInput.mTexCoord * 10.0f, 1.0f) );
	const float4 texel2 = gLayerMapArray.Sample( gTexturesSampler, float3(psInput.mTexCoord * 10.0f, 2.0f) );
	
	// Sample the blend map.
	const float4 blendMapTexel  = gBlendMap.Sample( gTexturesSampler, psInput.mTexCoord ); 
    
    // Blend the layers on top of each other.
    float4 texColor = texel0;
    texColor = lerp(texColor, texel1, blendMapTexel.r);
    texColor = lerp(texColor, texel2, blendMapTexel.b);
    
	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 ambientContribution;
    float4 diffuseContribution;
    float4 specularContribution;

    //
	// Estimate normal and tangent using central differences.
	//
	const float2 leftTexel = psInput.mTexCoord + float2(-gTexelCellSpaceU, 0.0f);
	const float2 rightTexel = psInput.mTexCoord + float2(gTexelCellSpaceU, 0.0f);
	const float2 bottomTexel = psInput.mTexCoord + float2(0.0f, gTexelCellSpaceV);
	const float2 topTexel = psInput.mTexCoord + float2(0.0f, -gTexelCellSpaceV);
	
	const float leftTexelHeight = gHeightMap.SampleLevel(gHeightMapSampler, leftTexel, 0).r;
	const float rightTexelHeight  = gHeightMap.SampleLevel(gHeightMapSampler, rightTexel, 0).r;
	const float bottomTexelHeight = gHeightMap.SampleLevel(gHeightMapSampler, bottomTexel, 0).r;
	const float topTexelHeight = gHeightMap.SampleLevel(gHeightMapSampler, topTexel, 0).r;
	
	const float3 tangent = normalize(float3(2.0f * gWorldCellSpace, rightTexelHeight - leftTexelHeight, 0.0f));
	const float3 bitangent = normalize(float3(0.0f, bottomTexelHeight - topTexelHeight, 2.0f * gWorldCellSpace)); 
	const float3 normalW = cross(tangent, bitangent);

    [unroll]
    for(int i = 0; i < 3; ++i)
    {
        computeDirectionalLight(gMaterial, 
                                gDirectionalLight[i], 
                                normalW, 
                                toEyeW, 
                                ambientContribution, 
                                diffuseContribution, 
                                specularContribution);
	    
        ambient += ambientContribution;  
	    diffuse += diffuseContribution;
	    specular += specularContribution;
    }
	   
	float4 finalColor = texColor * (ambient + diffuse) + specular;   

	// Common to take alpha from diffuse material and texture.
	finalColor.a = gMaterial.mDiffuse.a * texColor.a;

    return finalColor;
}