Texture2D heightMap : register(t0);
RWTexture2D<float4>	gGroupResultsTexture : register(u0);

#define GROUP_WIDTH 16
#define GROUP_HEIGHT 16

//
// Shared states
//
groupshared float gGroupResults[GROUP_WIDTH * GROUP_HEIGHT];
groupshared float4 gPlane;
groupshared float3 gNormals[2][2];
groupshared float3 gCorners[2][2];

//
// Auxiliar functions
//
float4 createPlaneFromPointAndNormal(in const float3 normal, in const float3 planePoint)
{
	return float4(normal, (-normal.x * planePoint.x - normal.y * planePoint.y - normal.z * planePoint.z));
}

float computeDistanceFromPlane(in const float4 plane, in const float3 position)
{
	return dot(plane.xyz, position) - plane.w;
}

//
// Main
//
[numthreads(GROUP_WIDTH, GROUP_HEIGHT, 1)]
void main(in const uint3 groupId : SV_GroupID, 
          in const uint3 groupThreadId : SV_GroupThreadID,
          in const uint3 dispatchThreadId : SV_DispatchThreadID, 
          in const uint groupIndex : SV_GroupIndex)
{   
	// groupId  = The xyz index as spun up by the actual application call
	//		      e.g. if ::Dispatch(GROUP_WIDTH, GROUP_HEIGHT, 1) is used then this will index into those bounds
    //
    // groupThreadId = The offset within the [numthreads()] bounds e.g. 0 - (GROUP_WIDTH - 1), 0 - (GROUP_HEIGHT - 1),0
	//
    // dispatchThreadId = Similar to groupId, but the global offset, so for [numthreads(GROUP_WIDTH,GROUP_HEIGHT,1)] and
	//                    ::Dispatch(GROUP_WIDTH,GROUP_HEIGHT,1) this variable will range between 0 - (GROUP_WIDTH*GROUP_HEIGHT - 1), 0 - (GROUP_WIDTH*GROUP_HEIGHT - 1), 0
    //
	// groupIndex = The flattened offset for this group e.g. 0 - (GROUP_WIDTH*GROUP_HEIGHT - 1) (GROUP_WIDTH*GROUP_HEIGHT)
	
    //
    // Step 1: For pixels located in the corners, store its
    //         height map sample and compute corners
    //
	if( ((groupThreadId.x ==  0) && (groupThreadId.y ==  0))
		||
		((groupThreadId.x == GROUP_WIDTH - 1) && (groupThreadId.y ==  0))
		||
		((groupThreadId.x ==  0) && (groupThreadId.y == GROUP_HEIGHT - 1))
		||
		((groupThreadId.x == GROUP_WIDTH - 1) && (groupThreadId.y == GROUP_HEIGHT - 1))
	  )
	{
		// This is a corner thread, so we want it to load
		// its value first
		gGroupResults[groupIndex] = heightMap.Load(uint3(dispatchThreadId.xy, 0)).r;
		const uint cornerIndexX = groupThreadId.x / (GROUP_WIDTH - 1);
        const uint cornerIndexY = groupThreadId.y / (GROUP_HEIGHT - 1);
		gCorners[cornerIndexX][cornerIndexY] = float3(cornerIndexX, gGroupResults[groupIndex], cornerIndexY);
		
		// The above will unfairly bias based on the height ranges
		gCorners[cornerIndexX][cornerIndexY].x /= 64.0f;
		gCorners[cornerIndexX][cornerIndexY].z /= 64.0f;
	}

	// Block until all threads have finished reading
	GroupMemoryBarrierWithGroupSync();
	
	//
    // Step 2: For pixels located at the corners, compute 
    //         its normal vector (using its neighbors)
    //         For other pixels, store its height map sample.
    //
	if((groupThreadId.x == 0) && (groupThreadId.y == 0))
	{
		gNormals[0][0] = normalize(cross
							(
								gCorners[0][1] - gCorners[0][0],
								gCorners[1][0] - gCorners[0][0]
							));
	}
	else if((groupThreadId.x == GROUP_WIDTH - 1) && (groupThreadId.y == 0))
	{
		gNormals[1][0] = normalize(cross
							(
								gCorners[0][0] - gCorners[1][0],
								gCorners[1][1] - gCorners[1][0]
							));
	}
	else if((groupThreadId.x == 0) && (groupThreadId.y == GROUP_HEIGHT - 1))
	{
		gNormals[0][1] = normalize(cross
							(
								gCorners[1][1] - gCorners[0][1],
								gCorners[0][0] - gCorners[0][1]
							));
	}
	else if((groupThreadId.x == GROUP_WIDTH - 1) && (groupThreadId.y == GROUP_HEIGHT - 1))
	{
		gNormals[1][1] = normalize(cross
							(
								gCorners[1][0] - gCorners[1][1],
								gCorners[0][1] - gCorners[1][1]
							));
	}
	else
	{
		// This is just one of the other threads, so let it
		// load in its sample into shared memory
		gGroupResults[groupIndex] = heightMap.Load(uint3( dispatchThreadId.xy, 0 )).r;
	}

	// Block until all the data is ready
	GroupMemoryBarrierWithGroupSync();
	
	//
    // Step 3: Let the first thread only determine the plane coefficients
    //
	if(groupIndex == 0)
	{	
		// Compute average normal vector
		const float3 planeNormal = normalize(gNormals[0][0] + gNormals[0][1] + gNormals[1][0] + gNormals[1][1]);
		
		// Compute the lowest point on which to base it
		float3 lowestPoint = float3(0.0f, 1e9f, 0.0f);
		for(uint i = 0; i < 2; ++i)
        {
			for(uint j = 0; j < 2; ++j)
            {
				if(gCorners[i][j].y < lowestPoint.y)
                {
					lowestPoint = gCorners[i][j];
                }
            }
        }
		
		// Derive the plane 
		gPlane = createPlaneFromPointAndNormal(planeNormal, lowestPoint);
	}

	GroupMemoryBarrierWithGroupSync();

    //
    // Step 4: For each pixel, using the stored height map
    //         sample, compute the distance to the base plane
    //         and store this instead.
    //

	// All threads now translate the raw height into the distance
	// from the base plane.
    const float3 heightPoint = float3((float)groupThreadId.x / GROUP_WIDTH, gGroupResults[groupIndex], (float)groupThreadId.y / GROUP_HEIGHT);
	gGroupResults[groupIndex] = computeDistanceFromPlane(gPlane, heightPoint);

	GroupMemoryBarrierWithGroupSync();
	
    //
    // Step 5: Compute standard deviation for this patch
    //
	if(groupIndex == 0)
	{
		// Let the first thread compute the standard deviation for
		// this patch. The 'average' is really just going to be 0.0
		// as we want the deviation from the plane and any point on the
		// plane now has a 'height' of zero.
		float standardDeviation = 0.0f;
		
        const float groupDimension = GROUP_WIDTH * GROUP_HEIGHT;
		for(int i = 0; i < groupDimension; ++i)
        {
			standardDeviation += pow(gGroupResults[i], 2);
        }
			
		standardDeviation /= groupDimension - 1.0f;
		
		standardDeviation = sqrt(standardDeviation);
		
		// Then write the normal vector and standard deviation
		// to the output buffer for use by the Domain and Hull Shaders
        const uint2 groupTextureIndex = uint2(groupId.x, groupId.y);
		gGroupResultsTexture[groupTextureIndex] = float4(gPlane.xyz, standardDeviation);
	}
}