cbuffer cbPerFrame : register(b0)
{
    float4x4 gWorldInverseTranspose;
	float4x4 gViewProjection;
    float2 gHeightMapPixelWidthHeight;
};

struct DSInput 
{
    float3 mPositionW : POSITION;
    float2 mTexCoord : TEXCOORD;
};

struct DSOutput
{
    float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
	float2 mTexCoord : TEXCOORD;
};

struct HSPerPatchOutput
{
	float mEdgeTessFactor[4] : SV_TessFactor; 
	float mInsideTessFactor[2] : SV_InsideTessFactor;
};

SamplerState heightMapSampler : register(s0);

Texture2D gHeightMap : register(t0);

#define NUM_OUTPUT_CONTROL_POINTS 4

float sampleHeightMap(in const float2 uv)
{
    // We should use SampleLevel() so e can specify the mip-map.
    // Domain shader has no gradient information it can use to derive this detail
    const float height = gHeightMap.SampleLevel(heightMapSampler, uv, 0.0f).b;
        
    // Arbitrary bias to make output more aesthetically pleasing
    const float biasScaleFactor = 50.0f;
    
    return biasScaleFactor * height;
}

float3 sobel(in const float2 texCoord )
{	
	// Compute the necessary offsets:
    // 00  01  02 
    // 10 [11] 12
    // 20  21  22
	const float2 upperLeftTexCoord = texCoord + float2(-gHeightMapPixelWidthHeight.x, -gHeightMapPixelWidthHeight.y);
	const float2 leftTexCoord = texCoord + float2(0.0f, -gHeightMapPixelWidthHeight.y );
	const float2 lowerLeftTexCoord = texCoord + float2(gHeightMapPixelWidthHeight.x, -gHeightMapPixelWidthHeight.y);
    const float2 upperTexCoord = texCoord + float2(-gHeightMapPixelWidthHeight.x, 0.0f);
	const float2 lowerTexCoord = texCoord + float2(gHeightMapPixelWidthHeight.x, 0.0f);
	const float2 upperRightTexCoord = texCoord + float2(-gHeightMapPixelWidthHeight.x,  gHeightMapPixelWidthHeight.y);
	const float2 rightTexCoord = texCoord + float2(0.0f, gHeightMapPixelWidthHeight.y);
	const float2 lowerRightTexCoord = texCoord + float2(gHeightMapPixelWidthHeight.x, gHeightMapPixelWidthHeight.y);

	// Use of the sobel filter requires the eight samples
	// surrounding the current pixel:
	const float upperLeftHeight = sampleHeightMap(upperLeftTexCoord);
	const float leftHeight = sampleHeightMap(leftTexCoord);
	const float lowerLeftHeight = sampleHeightMap(lowerLeftTexCoord);
	const float upperHeight = sampleHeightMap(upperTexCoord);
	const float lowerHeight = sampleHeightMap(lowerTexCoord);
	const float upperRightHeight = sampleHeightMap(upperRightTexCoord);
	const float rightHeight = sampleHeightMap(rightTexCoord);
	const float lowerRightHeight = sampleHeightMap(lowerRightTexCoord);

	// Evaluate the Sobel filters
	const float Gx = upperLeftHeight - upperLeftHeight + 2.0f * upperHeight - 2.0f * lowerHeight + upperRightHeight - lowerRightHeight;
	const float Gy = upperLeftHeight + 2.0f * leftHeight + lowerLeftHeight - upperRightHeight - 2.0f * rightHeight - lowerRightHeight;

	// Generate the missing Z
	const float Gz = 0.01f * sqrt( max(0.0f, 1.0f - Gx * Gx - Gy * Gy ) );

	// Make sure the returned normal is of unit length
	return normalize( float3( 2.0f * Gx, Gz, 2.0f * Gy ) );
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

    // Perform a sobel filter on the heightmap to determine 
    // an appropriate normal vector
	const float3 normal = sobel(texCoord);
    const float4 extendedNormal = float4(normal, 1.0f);
	const float3 normalW = normalize( mul(extendedNormal, gWorldInverseTranspose).xyz );

    // Determine the height from the texture
    positionW.y = sampleHeightMap(texCoord);

    // We then need to transform the world space coordinate
    // to be a proper projection space output
    // that the rasterizer an deal with.
    const float4 extendedPositionW = float4(positionW, 1.0f);
    dsOutput.mPositionH = mul(extendedPositionW, gViewProjection);
    dsOutput.mPositionW = positionW;
    dsOutput.mNormalW = normal;
    dsOutput.mTexCoord = texCoord;

    return dsOutput;
}