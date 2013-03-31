cbuffer cbPerFrame : register(b0)
{
	float3 gEyePositionW;
};

struct HSInput
{
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float3 mTangentW : TANGENT;
    float2 mTexCoord : TEXCOORD;
};

struct HSOutput
{
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

#define NUM_INPUT_CONTROL_POINTS 12
#define NUM_OUTPUT_CONTROL_POINTS 4
#define MIN_DISTANCE 0
#define MAX_DISTANCE 2000
#define MIN_MAX_DIFFERENCE_INVERTED (1.0f / (MAX_DISTANCE - MIN_DISTANCE))

float3 computePatchMidPoint(const float3 point0, const float3 point1, const float3 point2, const float3 point3)
{
    return (point0 + point1 + point2 + point3) * 0.25f;
}

float computeScaledDistance(const float3 from, const float3 to)
{
    // Compute the raw distance from the camera to the midpoint ot this patch
    const float fromToDistance = distance(from, to);

    // Scale this to be 0.0 (at the min distance) and 1.0 (at the max distance)
    return (fromToDistance - MIN_DISTANCE) * MIN_MAX_DIFFERENCE_INVERTED;
}

float computePatchLOD(const float3 midPoint) 
{
    // Compute the scaled distance
    const float scaledDistance = computeScaledDistance(gEyePositionW, midPoint);

    // Transform this 0.0 - 1.0 distance scale into the desired LOD's
    // Note: Invert the distance so that close is high detail and far is low detail
    return lerp(MIN_DISTANCE, MAX_DISTANCE, 1.0f - scaledDistance);
}

// Patch Constant Function
HSPerPatchOutput hsPerPatch(in const InputPatch<HSInput, NUM_INPUT_CONTROL_POINTS> inputPatch,
                            in const uint patchID : SV_PrimitiveID)
{
	HSPerPatchOutput hsPerPatchOutput = (HSPerPatchOutput)0;

    // Determine the mid point of this patch
    float3 midPoints[] =
    {
        // Main quad
        computePatchMidPoint(inputPatch[0].mPositionW, inputPatch[1].mPositionW, inputPatch[2].mPositionW, inputPatch[3].mPositionW),

        // +z neighbor
        computePatchMidPoint(inputPatch[2].mPositionW, inputPatch[3].mPositionW, inputPatch[4].mPositionW, inputPatch[5].mPositionW),

        // +x neighbor
        computePatchMidPoint(inputPatch[1].mPositionW, inputPatch[6].mPositionW, inputPatch[3].mPositionW, inputPatch[7].mPositionW),

        // -z neighbor
        computePatchMidPoint(inputPatch[8].mPositionW, inputPatch[9].mPositionW, inputPatch[0].mPositionW, inputPatch[1].mPositionW),

        // -x neighbor
        computePatchMidPoint(inputPatch[10].mPositionW, inputPatch[0].mPositionW, inputPatch[11].mPositionW, inputPatch[2].mPositionW)
    };

    // Determine the appropiate LOD for this patch.
    float distances[] =
    {
        // Main quad
        computePatchLOD(midPoints[0]),

        // +z neighbor
        computePatchLOD(midPoints[1]),

        // +x neighbor
        computePatchLOD(midPoints[2]),

        // -z neighbor
        computePatchLOD(midPoints[3]),

        // -x neighbor
        computePatchLOD(midPoints[4])
    };

    // Set it up so that this patch always has an interior matching 
    // the patch LOD
    const float distancesAverage = (distances[0] + distances[1] + distances[2] + distances[3]) * 0.25f;
    hsPerPatchOutput.mInsideTessFactor[0] = distancesAverage;
    hsPerPatchOutput.mInsideTessFactor[1] = distancesAverage;

    // For the edges its more complex as we have to match
    // the neighboring patches. The rule in this case is:
    // - If the neighbor patch is of lower LOD we
    //   pick that LOD as the edge for this patch.
    //
    // - If the neighbor patch is a higher LOD then
    //   we stick with our LOD and expect them to blend down
    //   towards up.
    hsPerPatchOutput.mEdgeTessFactor[0] = min(distances[0], distances[3]);
    hsPerPatchOutput.mEdgeTessFactor[1] = min(distances[0], distances[4]);
    hsPerPatchOutput.mEdgeTessFactor[2] = min(distances[0], distances[1]);
    hsPerPatchOutput.mEdgeTessFactor[3] = min(distances[0], distances[2]);

	return hsPerPatchOutput;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_OUTPUT_CONTROL_POINTS)]
[patchconstantfunc("hsPerPatch")]
HSOutput main(in const InputPatch<HSInput, NUM_INPUT_CONTROL_POINTS> inputPatch,
              in const uint outputControlPointID : SV_OutputControlPointID)
{
	HSOutput hsOutput = (HSOutput)0;

    hsOutput.mPositionW = inputPatch[outputControlPointID].mPositionW;
	hsOutput.mNormalW = inputPatch[outputControlPointID].mNormalW;
    hsOutput.mTangentW = inputPatch[outputControlPointID].mTangentW;
    hsOutput.mTexCoord = inputPatch[outputControlPointID].mTexCoord;

	return hsOutput;
}
