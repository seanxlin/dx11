#include "Lights.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirectionalLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePositionW;
};

cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld;
	float4x4 gWorldInverseTranspose;
	float4x4 gWorldViewProjection;
    float4x4 gTexTransform;
	Material gMaterial;
};

SamplerState samplerLinear : register(s0);