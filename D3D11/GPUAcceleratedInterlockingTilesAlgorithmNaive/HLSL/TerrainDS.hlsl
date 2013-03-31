cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewProjection;
};

struct DSInput 
{
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

struct DSOutput
{
    float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
	float3 mTangentW : TANGENT;
	float2 mTexCoord : TEXCOORD;
};

struct HSPerPatchOutput
{
	float mEdgeTessFactor[4] : SV_TessFactor; 
	float mInsideTessFactor[2] : SV_InsideTessFactor;
};

SamplerState mySampler : register(s0);

Texture2D gHeightMap : register(t0);

#define NUM_OUTPUT_CONTROL_POINTS 4

float sampleHeightMap(in const float2 uv)
{
    // We should use SampleLevel() so e can specify the mip-map.
    // Domain shader has no gradient information it can use to derive this detail
    const float height = gHeightMap.SampleLevel(mySampler, uv, 0.0f).r;
        
    // Arbitrary bias to make output more aesthetically pleasing
    const float biasScaleFactor = 3.0f;
    
    return biasScaleFactor * height;
}

[domain("quad")]
DSOutput main(in const HSPerPatchOutput hsPerPatchOutput,
              in const float2 domainLocation : SV_DomainLocation,
              in const OutputPatch<DSInput, NUM_OUTPUT_CONTROL_POINTS> patch)
{
    DSOutput dsOutput = (DSOutput)0;

    // We need to take the three world space coordinates
    // in patch and the interpolation values in uvw (barycentric coords)
    // and determine the appropiate interpolated position
    float3 positionW = float3(0.0f, 0.0f, 0.0f);

    // u, v
    // 0, 0 is patch[0] position
    // 1, 0 is patch[1] position
    // 0, 1 is patch[2] position
    // 1, 1 is patch[3] position
    //0 --- 1
    //    /
    //   /
    //  /
    //2 --- 3
    positionW.xz = patch[0].mPositionW.xz * (1.0f - domainLocation.x) * (1.0f - domainLocation.y)
                 + patch[1].mPositionW.xz * domainLocation.x * (1.0f - domainLocation.y)
                 + patch[2].mPositionW.xz * (1.0f - domainLocation.x) * domainLocation.y
                 + patch[3].mPositionW.xz * domainLocation.x * domainLocation.y;

    const float2 texCoord = patch[0].mTexCoord * (1.0f - domainLocation.x) * (1.0f - domainLocation.y)
                          + patch[1].mTexCoord * domainLocation.x * (1.0f - domainLocation.y)
                          + patch[2].mTexCoord * (1.0f - domainLocation.x) * domainLocation.y
                          + patch[3].mTexCoord * domainLocation.x * domainLocation.y;

    float3 normalW = patch[0].mNormalW * (1.0f - domainLocation.x) * (1.0f - domainLocation.y)
                         + patch[1].mNormalW * domainLocation.x * (1.0f - domainLocation.y)
                         + patch[2].mNormalW * (1.0f - domainLocation.x) * domainLocation.y
                         + patch[3].mNormalW * domainLocation.x * domainLocation.y;

    normalW = normalize(normalW);

    const float3 tangentW = patch[0].mTangentW * (1.0f - domainLocation.x) * (1.0f - domainLocation.y)
                          + patch[1].mTangentW * domainLocation.x * (1.0f - domainLocation.y)
                          + patch[2].mTangentW * (1.0f - domainLocation.x) * domainLocation.y
                          + patch[3].mTangentW * domainLocation.x * domainLocation.y;

    // Determine the height from the texture
    positionW.y = sampleHeightMap(texCoord);

    // We then need to transform the world space coordinate
    // to be a proper projection space output
    // that the rasterizer an deal with.
    const float4 extendedPositionW = float4(positionW, 1.0f);
    dsOutput.mPositionH = mul(extendedPositionW, gViewProjection);
    dsOutput.mPositionW = positionW;
    dsOutput.mNormalW = normalW;
    dsOutput.mTangentW = tangentW;
    dsOutput.mTexCoord = texCoord;

    return dsOutput;
}