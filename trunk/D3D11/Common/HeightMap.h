//////////////////////////////////////////////////////////////////////////
//
// In computer graphics, a heightmap or heightfield is a raster image used 
// to store values, such as surface elevation data, for display 
// in 3D computer graphics. A heightmap can be used in bump mapping to 
// calculate where this 3D data would create shadow in a material, 
// in displacement mapping to displace the actual geometric position of 
// points over the textured surface, or for terrain where the heightmap is 
// converted into a 3D mesh. 
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ID3D11Device;
struct ID3D11ShaderResourceView;

struct HeightMap
{
    HeightMap(const uint32_t dimension)
        : mData(dimension * dimension, 0.0f) 
        , mDimension(dimension)   
    {
        mData.resize(mDimension * mDimension);
    }

    std::vector<float> mData;
    uint32_t mDimension;
};

namespace HeightMapUtils
{
    void loadFromRAWFile(const std::string& filePath, 
                         const float scaleFactor,
                         HeightMap& heightMap);

    // Apply a filter to make height map smoother
    // taking into account its neighbors pixels.
    void applyNeighborsFilter(HeightMap& heightMap);

    ID3D11ShaderResourceView* buildSRV(ID3D11Device& device,
                                       const HeightMap& heightMap,
                                       const uint32_t texture2DDescBindFlags);
}