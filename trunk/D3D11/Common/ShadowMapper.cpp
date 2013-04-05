#include "ShadowMapper.h"

#include <DxErrorChecker.h>

namespace Utils
{
    ShadowMapper::ShadowMapper(ID3D11Device& device, const uint32_t width, const uint32_t height)
        : mWidth(width)
        , mHeight(height)
        , mDepthMapSRV(nullptr)
        , mDepthMapDSV(nullptr)
    {
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(width);
        mViewport.Height = static_cast<float>(height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        // Use typeless format because the DSV is going to interpret
        // the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
        // the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
        D3D11_TEXTURE2D_DESC textureDescription;
        textureDescription.Width = mWidth;
        textureDescription.Height = mHeight;
        textureDescription.MipLevels = 1;
        textureDescription.ArraySize = 1;
        textureDescription.Format = DXGI_FORMAT_R24G8_TYPELESS;
        textureDescription.SampleDesc.Count   = 1;  
        textureDescription.SampleDesc.Quality = 0;  
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        textureDescription.CPUAccessFlags = 0; 
        textureDescription.MiscFlags = 0;

        ID3D11Texture2D* depthMap = nullptr;
        HRESULT result = device.CreateTexture2D(&textureDescription, 0, &depthMap);
        DxErrorChecker(result);

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        depthStencilViewDesc.Flags = 0;
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;
        result = device.CreateDepthStencilView(depthMap, &depthStencilViewDesc, &mDepthMapDSV);
        DxErrorChecker(result);

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MipLevels = textureDescription.MipLevels;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        result = device.CreateShaderResourceView(depthMap, &shaderResourceViewDesc, &mDepthMapSRV);
        DxErrorChecker(result);

        // View saves a reference to the texture so we can release our reference.
        depthMap->Release();
    }

    ShadowMapper::~ShadowMapper()
    {
        mDepthMapSRV->Release();
        mDepthMapDSV->Release();
    }

    ID3D11ShaderResourceView& ShadowMapper::depthMapSRV()
    {
        return *mDepthMapSRV;
    }

    void ShadowMapper::bindDepthStencilViewAndSetNullRenderTarget(ID3D11DeviceContext& deviceContext)
    {
        deviceContext.RSSetViewports(1, &mViewport);

        // Set null render target because we are only going to draw to depth buffer.
        // Setting a null render target will disable color writes.
        ID3D11RenderTargetView* renderTargets[1] = { nullptr };
        deviceContext.OMSetRenderTargets(1, renderTargets, mDepthMapDSV);

        deviceContext.ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }


}