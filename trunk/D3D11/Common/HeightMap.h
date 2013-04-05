#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ID3D11Device;
struct ID3D11ShaderResourceView;

struct HeightMap
{
    HeightMap(const uint32_t dimension)
        : mDimension(dimension)
    {
        mData.resize(mDimension * mDimension);
    }

    std::vector<float> mData;
    uint32_t mDimension;
};

namespace HeightMapUtils
{
    // Load a height map from a RAW file and stores its content
    // in a vector.
    void loadHeightMapFromRAWFile(const std::string& filePath, 
                                  const float scaleFactor,
                                  HeightMap& heightMap);

    // Apply a filter to make height map more smooth
    // taking into account its neighbors pixels.
    void applyNeighborsFilter(HeightMap& heightMap);

    ID3D11ShaderResourceView* buildHeightMapSRV(ID3D11Device& device,
                                                const HeightMap& heightMap,
                                                const uint32_t texture2DDescBindFlags);
}