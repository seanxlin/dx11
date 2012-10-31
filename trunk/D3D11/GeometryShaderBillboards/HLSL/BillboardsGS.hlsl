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

struct GeometryShaderInput
{
    float3 mCenterW : POSITION;
    float2 mSizeW : SIZE;
};

struct GeometryShaderOutput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
    uint mPrimitiveID : SV_PrimitiveID;
};


// We expand each point into a quad (4 vertices), so the maximum number of vertices
 // we output per geometry shader invocation is 4.
[maxvertexcount(4)]
void main(point GeometryShaderInput input[1], uint primitiveID : SV_PrimitiveID, 
        inout TriangleStream<GeometryShaderOutput> triangleStream)
{	
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePositionW - input[0].mCenterW;
	look.y = 0.0f; // y-axis aligned, so project to xz-plane
	look = normalize(look);
	float3 left = cross(up, look);

	// Compute triangle strip vertices (quad) in world space.
	float halfWidth  = 0.5f * input[0].mSizeW.x;
	float halfHeight = 0.5f * input[0].mSizeW.y;
	
    float4 bottomLeft =  float4(input[0].mCenterW + halfWidth * left - halfHeight * up, 1.0f);
	float4 topLeft = float4(input[0].mCenterW + halfWidth * left + halfHeight * up, 1.0f);
    float4 bottomRight = float4(input[0].mCenterW - halfWidth * left - halfHeight * up, 1.0f);
    float4 topRight = float4(input[0].mCenterW - halfWidth * left + halfHeight * up, 1.0f);
    
    float4 v[4] = { bottomLeft, topLeft, bottomRight, topRight };
	
    // Transform quad vertices to world space and output 
	// them as a triangle strip.
	GeometryShaderOutput output;
	
    output.mPositionH = mul(v[0], gViewProjection);
	output.mPositionW = v[0].xyz;
	output.mNormalW = look;
	output.mTexCoord = float2(0.0f, 1.0f);
	output.mPrimitiveID = primitiveID;		
	triangleStream.Append(output);

    output.mPositionH = mul(v[1], gViewProjection);
    output.mPositionW = v[1].xyz;
	output.mNormalW = look;
	output.mTexCoord = float2(0.0f, 0.0f);
	output.mPrimitiveID = primitiveID;		
	triangleStream.Append(output);

    output.mPositionH = mul(v[2], gViewProjection);
	output.mPositionW = v[2].xyz;
	output.mNormalW = look;
	output.mTexCoord = float2(1.0f, 1.0f);
	output.mPrimitiveID = primitiveID;		
	triangleStream.Append(output);

    output.mPositionH = mul(v[3], gViewProjection);
	output.mPositionW = v[3].xyz;
	output.mNormalW = look;
	output.mTexCoord = float2(1.0f, 0.0f);
	output.mPrimitiveID = primitiveID;		
	triangleStream.Append(output);
}