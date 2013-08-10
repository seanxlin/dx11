#include "ShaderResourcesManager.h"

#include <cassert>
#include <fstream>
#include <D3D11.h>
#include <vector>

#include <DDSTextureLoader.h>
#include <DxErrorChecker.h>
#include <HeightMap.h>

namespace
{
    ID3D11ShaderResourceView* createTexture2DArraySRV(ID3D11Device& device, 
                                                      ID3D11DeviceContext& context,
                                                      const std::vector<std::wstring>& filenames) 
    {
        //
        // Load the texture elements individually from file.  These textures
        // won't be used by the GPU (0 bind flags), they are just used to 
        // load the image data from file.  We use the STAGING usage so the
        // CPU can read the resource.
        //

        const uint32_t numFilenames = static_cast<uint32_t> (filenames.size());

        std::vector<ID3D11Texture2D*> sourceTextures;
        sourceTextures.resize(numFilenames);
        for(uint32_t filenameIndex = 0; filenameIndex < numFilenames; ++filenameIndex) {
            ID3D11Texture2D * &resource = sourceTextures[filenameIndex];
            ID3D11Resource* &texture = reinterpret_cast<ID3D11Resource*&> (resource);

            ID3D11ShaderResourceView* shaderResourceView;
            HRESULT result = CreateDDSTextureFromFile(&device, 
                                                      filenames[filenameIndex].c_str(),
                                                      &texture, 
                                                      &shaderResourceView);
            DxErrorChecker(result);

            shaderResourceView->Release();
        }

        //
        // Create the texture array.  Each element in the texture 
        // array has the same format/dimensions.
        //

        D3D11_TEXTURE2D_DESC textureElementDesc;
        sourceTextures[0]->GetDesc(&textureElementDesc);

        D3D11_TEXTURE2D_DESC textureArrayDesc;
        textureArrayDesc.Width = textureElementDesc.Width;
        textureArrayDesc.Height = textureElementDesc.Height;
        textureArrayDesc.MipLevels = textureElementDesc.MipLevels;
        textureArrayDesc.ArraySize = numFilenames;
        textureArrayDesc.Format = textureElementDesc.Format;
        textureArrayDesc.SampleDesc.Count = 1;
        textureArrayDesc.SampleDesc.Quality = 0;
        textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
        textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureArrayDesc.CPUAccessFlags = 0;
        textureArrayDesc.MiscFlags = 0;

        ID3D11Texture2D* textureArray;
        HRESULT result = device.CreateTexture2D(&textureArrayDesc, 0, &textureArray);
        DxErrorChecker(result);

        D3D11_TEXTURE2D_DESC textureElementDesc2;
        textureArray->GetDesc(&textureElementDesc2);
        

        //
        // Copy individual texture elements into texture array.
        //

        // for each texture element...
        const size_t mipLevels = textureElementDesc.MipLevels; 
        for (uint32_t filenameIndex = 0; filenameIndex < numFilenames; ++filenameIndex) {
            // for each mipmap level...
            for(uint32_t mipLevelIndex = 0; 
                         mipLevelIndex < mipLevels; 
                         ++mipLevelIndex) {
                const uint32_t subResourceIndex = D3D11CalcSubresource(mipLevelIndex, 
                                                                       0, 
                                                                       mipLevels);
                const uint32_t destinationSubresource = 
                    D3D11CalcSubresource(mipLevelIndex,
                                         filenameIndex,
                                         mipLevels);
                context.CopySubresourceRegion(textureArray, 
                                              static_cast<uint32_t> (destinationSubresource), 
                                              0, 
                                              0, 
                                              0, 
                                              sourceTextures[filenameIndex], 
                                              subResourceIndex, 
                                              nullptr);
            }
        }	

        // Create a resource view to the texture array.
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = textureArrayDesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        viewDesc.Texture2DArray.MipLevels = textureArrayDesc.MipLevels;
        viewDesc.Texture2DArray.FirstArraySlice = 0;
        viewDesc.Texture2DArray.ArraySize = numFilenames;

        ID3D11ShaderResourceView* textureArraySRV;
        result = device.CreateShaderResourceView(textureArray, &viewDesc, &textureArraySRV);
        DxErrorChecker(result);

        // Cleanup--we only need the resource view.
        textureArray->Release();

        for(size_t filenameIndex = 0; filenameIndex < numFilenames; ++filenameIndex) {
            sourceTextures[filenameIndex]->Release();
        }

        return textureArraySRV;
    }
}

ShaderResources::ShaderResources()
    : mHeightMapSRV(nullptr)
    , mTerrainDiffuseMapArraySRV(nullptr)
    , mTerrainBlendMapSRV(nullptr)
{

}

namespace ShaderResourcesUtils
{    
    void initAll(ID3D11Device& device, 
                 ID3D11DeviceContext& context, 
                 ShaderResources& shaderResources)
    {
        assert(shaderResources.mHeightMapSRV == nullptr);
        assert(shaderResources.mTerrainDiffuseMapArraySRV == nullptr);
        assert(shaderResources.mTerrainBlendMapSRV == nullptr);

        ID3D11Resource* texture;

        // Height map
        const uint32_t heightMapDimension = 512;
        HeightMap heightMap(heightMapDimension);
        const float heightMapScaleFactor = 150.0f;
        HeightMapUtils::loadFromRAWFile("Resources/Textures/terrainRaw.raw",
                                        heightMapScaleFactor, 
                                        heightMap);

        HeightMapUtils::applyNeighborsFilter(heightMap);
        shaderResources.mHeightMapSRV = HeightMapUtils::buildSRV(device,
                                                                 heightMap,
                                                                 D3D11_BIND_SHADER_RESOURCE);

        // Create terrain textures array.
        std::vector<std::wstring> texturesFilenames;
        texturesFilenames.push_back(L"Resources/Textures/grass.dds");
        texturesFilenames.push_back(L"Resources/Textures/lightdirt.dds");
        texturesFilenames.push_back(L"Resources/Textures/darkdirt.dds");
        shaderResources.mTerrainDiffuseMapArraySRV = createTexture2DArraySRV(
            device,
            context,
            texturesFilenames
        ); 

        // Blend map
        const HRESULT result = CreateDDSTextureFromFile(&device, 
                                                        L"Resources/Textures/blend.dds", 
                                                        &texture,
                                                        &shaderResources.mTerrainBlendMapSRV);
        DxErrorChecker(result);  

        texture->Release();
    }
    
    void destroyAll(ShaderResources& shaderResources)
    {
        assert(shaderResources.mHeightMapSRV);
        assert(shaderResources.mTerrainDiffuseMapArraySRV);
        assert(shaderResources.mTerrainBlendMapSRV);

        shaderResources.mHeightMapSRV->Release();
        shaderResources.mTerrainDiffuseMapArraySRV->Release();
        shaderResources.mTerrainBlendMapSRV->Release();

    }
}
