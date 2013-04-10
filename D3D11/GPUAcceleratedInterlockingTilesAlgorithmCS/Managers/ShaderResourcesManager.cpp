#include "ShaderResourcesManager.h"

#include <cassert>
#include <fstream>
#include <D3D11.h>
#include <vector>

#include <DDSTextureLoader.h>
#include <DxErrorChecker.h>
#include <HeightMap.h>

#include <Main/Globals.h>

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

        const uint32_t size = static_cast<uint32_t> (filenames.size());

        std::vector<ID3D11Texture2D*> sourceTextures;
        sourceTextures.resize(size);
        for(uint32_t i = 0; i < size; ++i)
        {
            ID3D11ShaderResourceView* shaderResourceView =  nullptr;
            ID3D11Resource* resource =  nullptr;
            HRESULT result = CreateDDSTextureFromFile(&device, 
                                                      filenames[i].c_str(),
                                                      reinterpret_cast<ID3D11Resource**> (&sourceTextures[i]), 
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
        textureArrayDesc.ArraySize = size;
        textureArrayDesc.Format = textureElementDesc.Format;
        textureArrayDesc.SampleDesc.Count = 1;
        textureArrayDesc.SampleDesc.Quality = 0;
        textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
        textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureArrayDesc.CPUAccessFlags = 0;
        textureArrayDesc.MiscFlags = 0;

        ID3D11Texture2D* textureArray = 0;
        HRESULT result = device.CreateTexture2D(&textureArrayDesc, 0, &textureArray);
        DxErrorChecker(result);

        D3D11_TEXTURE2D_DESC textureElementDesc2;
        textureArray->GetDesc(&textureElementDesc2);
        const size_t ml = textureElementDesc2.MipLevels; 

        //
        // Copy individual texture elements into texture array.
        //

        // for each texture element...
        for (uint32_t texElement = 0; texElement < size; ++texElement)
        {
            // for each mipmap level...
            for(uint32_t mipLevel = 0; mipLevel < textureElementDesc.MipLevels; ++mipLevel)
            {
                const uint32_t subResourceIndex = D3D11CalcSubresource(mipLevel, 
                                                                       0, 
                                                                       textureElementDesc.MipLevels);
                const uint32_t destinationSubresource = D3D11CalcSubresource(mipLevel, 
                                                                             texElement, 
                                                                             textureElementDesc.MipLevels);
                context.CopySubresourceRegion(textureArray, 
                                              static_cast<uint32_t> (destinationSubresource), 
                                              0, 
                                              0, 
                                              0, 
                                              sourceTextures[texElement], 
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
        viewDesc.Texture2DArray.ArraySize = size;

        ID3D11ShaderResourceView* textureArraySRV = 0;
        result = device.CreateShaderResourceView(textureArray, &viewDesc, &textureArraySRV);
        DxErrorChecker(result);

        // Cleanup--we only need the resource view.
        textureArray->Release();

        for(size_t i = 0; i < size; ++i)
            sourceTextures[i]->Release();

        return textureArraySRV;
    }
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
        assert(shaderResources.mGroupResultsSRV == nullptr);
        assert(shaderResources.mGroupResultsUAV == nullptr);

        ID3D11Resource* texture = nullptr;

        //
        // Height map
        //
        const uint32_t heightMapDimension = 512;
        HeightMap heightMap(heightMapDimension);
        const float heightMapScaleFactor = 150.0f;
        HeightMapUtils::loadHeightMapFromRAWFile(
            "Resources/Textures/terrainRaw.raw",
            heightMapScaleFactor, 
            heightMap);

        HeightMapUtils::applyNeighborsFilter(heightMap);
        shaderResources.mHeightMapSRV = HeightMapUtils::buildHeightMapSRV(
            device,
            heightMap,
            D3D11_BIND_SHADER_RESOURCE);

        //
        // Create terrain textures array.
        //
        std::vector<std::wstring> texturesFilenames;
        texturesFilenames.push_back(L"Resources/Textures/grass.dds");
        texturesFilenames.push_back(L"Resources/Textures/lightdirt.dds");
        texturesFilenames.push_back(L"Resources/Textures/stone.dds");
        texturesFilenames.push_back(L"Resources/Textures/darkdirt.dds");
        texturesFilenames.push_back(L"Resources/Textures/snow.dds");
        shaderResources.mTerrainDiffuseMapArraySRV = createTexture2DArraySRV(
            device, 
            context, 
            texturesFilenames); 

        //
        // Blend map
        //
        HRESULT result = CreateDDSTextureFromFile(&device, 
            L"Resources/Textures/blend.dds", 
            &texture, 
            &shaderResources.mTerrainBlendMapSRV);
        DxErrorChecker(result);  
        
        texture->Release();

        //
        // Create texture to store the results
        //        
        D3D11_TEXTURE2D_DESC groupResultsTexDesc;
        groupResultsTexDesc.Width = 32;
        groupResultsTexDesc.Height = 32;
        groupResultsTexDesc.MipLevels = 1;
        groupResultsTexDesc.ArraySize = 1;
        groupResultsTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        groupResultsTexDesc.SampleDesc.Count   = 1;
        groupResultsTexDesc.SampleDesc.Quality = 0;
        groupResultsTexDesc.Usage = D3D11_USAGE_DEFAULT;
        groupResultsTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        groupResultsTexDesc.CPUAccessFlags = 0;
        groupResultsTexDesc.MiscFlags = 0;

        ID3D11Texture2D* groupResultsTex = nullptr;
        assert(Globals::gDirect3DData.mDevice);
        result = device.CreateTexture2D(&groupResultsTexDesc, 0, &groupResultsTex);
        DxErrorChecker(result);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        result = device.CreateTexture2D(&groupResultsTexDesc, 0, &groupResultsTex);
        result = device.CreateShaderResourceView(groupResultsTex, &srvDesc, &shaderResources.mGroupResultsSRV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        result = device.CreateUnorderedAccessView(groupResultsTex, &uavDesc, &shaderResources.mGroupResultsUAV);

        // Views save a reference to the texture so we can release our reference.
        groupResultsTex->Release();
    }
    
    void destroyAll(ShaderResources& shaderResources)
    {
        assert(shaderResources.mHeightMapSRV);
        assert(shaderResources.mTerrainDiffuseMapArraySRV);
        assert(shaderResources.mTerrainBlendMapSRV);
        assert(shaderResources.mGroupResultsSRV);
        assert(shaderResources.mGroupResultsUAV);

        shaderResources.mHeightMapSRV->Release();
        shaderResources.mTerrainDiffuseMapArraySRV->Release();
        shaderResources.mTerrainBlendMapSRV->Release();
        shaderResources.mGroupResultsSRV->Release();
        shaderResources.mGroupResultsUAV->Release();

    }
}
