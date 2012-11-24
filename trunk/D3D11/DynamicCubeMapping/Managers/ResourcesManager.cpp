#include "ResourcesManager.h"

#include <cassert>
#include <D3D11.h>
#include <vector>

#include <DDSTextureLoader.h>
#include <DxErrorChecker.h>

namespace
{
    ID3D11ShaderResourceView* createTexture2DArraySRV(ID3D11Device* device, ID3D11DeviceContext* context,
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
            HRESULT result = CreateDDSTextureFromFile(device, filenames[i].c_str(), 
                reinterpret_cast<ID3D11Resource**> (&sourceTextures[i]), &shaderResourceView);
            DebugUtils::DxErrorChecker(result);

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
        HRESULT result = device->CreateTexture2D(&textureArrayDesc, 0, &textureArray);
        DebugUtils::DxErrorChecker(result);

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
                const uint32_t subResourceIndex = D3D11CalcSubresource(mipLevel, 0, textureElementDesc.MipLevels);
                const uint32_t destinationSubresource = D3D11CalcSubresource(mipLevel, texElement, textureElementDesc.MipLevels);
                context->CopySubresourceRegion(textureArray, static_cast<uint32_t> (destinationSubresource), 0, 0, 0, sourceTextures[texElement], subResourceIndex, nullptr);
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
        result = device->CreateShaderResourceView(textureArray, &viewDesc, &textureArraySRV);
        DebugUtils::DxErrorChecker(result);

        // Cleanup--we only need the resource view.
        textureArray->Release();

        for(size_t i = 0; i < size; ++i)
            sourceTextures[i]->Release();

        return textureArraySRV;
    }
}

namespace Managers
{
    ID3D11ShaderResourceView* ResourcesManager::mSandSRV = nullptr;
    ID3D11ShaderResourceView* ResourcesManager::mSkyCubeMapSRV = nullptr;
    ID3D11ShaderResourceView* ResourcesManager::mSphereDiffuseMapSRV = nullptr;
    ID3D11DepthStencilView* ResourcesManager::mDynamicCubeMapDSV = nullptr;
    ID3D11RenderTargetView* ResourcesManager::mDynamicCubeMapRTV[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView* ResourcesManager::mDynamicCubeMapSRV = nullptr;
    
    void ResourcesManager::initAll(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        assert(device);
        assert(context);

        ID3D11Resource* texture = nullptr;

        // Create grass texture shader resource view
        HRESULT result = CreateDDSTextureFromFile(device, L"Resources/Textures/sand.dds", &texture, &mSandSRV);
        DebugUtils::DxErrorChecker(result);  

        // Create sky cube map shader resource view
        result = CreateDDSTextureFromFile(device, L"Resources/Textures/desertcube1024.dds", &texture, &mSkyCubeMapSRV);
        DebugUtils::DxErrorChecker(result);  

        // Create sphere diffuse map texture
        result = CreateDDSTextureFromFile(device, L"Resources/Textures/stone.dds", &texture, &mSphereDiffuseMapSRV);
        DebugUtils::DxErrorChecker(result);

        texture->Release();

        buildDynamicCubeMapViews(device);
    }
    
    void ResourcesManager::destroyAll()
    {
        mSandSRV->Release();
        mSkyCubeMapSRV->Release();
        mSphereDiffuseMapSRV->Release();

        mDynamicCubeMapDSV->Release();

        for(size_t i = 0; i < 6; ++i)
            mDynamicCubeMapRTV[i]->Release();

        mDynamicCubeMapSRV->Release();
    }

    void ResourcesManager::buildDynamicCubeMapViews(ID3D11Device* device)
    {
        assert(device);

        //
        // Cubemap is a special texture array with 6 elements.
        //
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width = 256;
        texDesc.Height = 256;
        texDesc.MipLevels = 0;
        texDesc.ArraySize = 6;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

        ID3D11Texture2D* cubeTex = nullptr;
        HRESULT result = device->CreateTexture2D(&texDesc, 0, &cubeTex);
        DebugUtils::DxErrorChecker(result);

        //
        // Create a render target view to each cube map face 
        // (i.e., each element in the texture array).
        // 
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        renderTargetViewDesc.Format = texDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        renderTargetViewDesc.Texture2DArray.ArraySize = 1;
        renderTargetViewDesc.Texture2DArray.MipSlice = 0;

        for(size_t i = 0; i < 6; ++i)
        {
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = static_cast<uint32_t> (i);
            result = device->CreateRenderTargetView(cubeTex, &renderTargetViewDesc, &mDynamicCubeMapRTV[i]);
            DebugUtils::DxErrorChecker(result);
        }

        // Create a shader resource view to the cube map.
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = texDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
        shaderResourceViewDesc.TextureCube.MipLevels = -1;

        result = device->CreateShaderResourceView(cubeTex, &shaderResourceViewDesc, &mDynamicCubeMapSRV);
        DebugUtils::DxErrorChecker(result);

        cubeTex->Release();

        // We need a depth texture for rendering the scene into the cubemap
        // that has the same resolution as the cubemap faces.  
        D3D11_TEXTURE2D_DESC depthTexDesc;
        depthTexDesc.Width = 256;
        depthTexDesc.Height = 256;
        depthTexDesc.MipLevels = 1;
        depthTexDesc.ArraySize = 1;
        depthTexDesc.SampleDesc.Count = 1;
        depthTexDesc.SampleDesc.Quality = 0;
        depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
        depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthTexDesc.CPUAccessFlags = 0;
        depthTexDesc.MiscFlags = 0;

        ID3D11Texture2D* depthTex = 0;
        result = device->CreateTexture2D(&depthTexDesc, 0, &depthTex);
        DebugUtils::DxErrorChecker(result);

        // Create the depth stencil view for the entire cube
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = depthTexDesc.Format;
        dsvDesc.Flags  = 0;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        result = device->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV);
        DebugUtils::DxErrorChecker(result);

        depthTex->Release();
    }
}
