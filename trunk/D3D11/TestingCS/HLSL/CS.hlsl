Texture2D heightMap : register(t0);
RWTexture2D<float4>	gGroupResultsTexture : register(u0);

#define GROUP_WIDTH 1
#define GROUP_HEIGHT 1

//
// Main
//
[numthreads(GROUP_WIDTH, GROUP_HEIGHT, 1)]
void main(in const uint3 groupId : SV_GroupID, 
          in const uint3 groupThreadId : SV_GroupThreadID,
          in const uint3 dispatchThreadId : SV_DispatchThreadID, 
          in const uint groupIndex : SV_GroupIndex)
{   
    //
    // Step 5: Compute standard deviation for this patch
    //
    const uint2 groupTextureIndex = uint2(dispatchThreadId.x, dispatchThreadId.y);
	gGroupResultsTexture[groupTextureIndex] = heightMap.Load(uint3(groupTextureIndex, 0));
}