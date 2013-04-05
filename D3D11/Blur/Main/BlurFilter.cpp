#include "BlurFilter.h"

#include <cmath>

#include <DxErrorChecker.h>

#include "Managers/ShadersManager.h"

namespace Framework
{
    ID3D11ShaderResourceView* BlurFilter::blurredOutput()
    {
        return mBlurredOutputTexSRV;
    }

    void BlurFilter::init(ID3D11Device* device, const uint32_t width, const uint32_t height, DXGI_FORMAT format)
    {
        // Start fresh.
        if (mBlurredOutputTexSRV)
            mBlurredOutputTexSRV->Release();

        if (mBlurredOutputTexUAV)
        mBlurredOutputTexUAV->Release();

        mWidth = width;
        mHeight = height;
        mFormat = format;

        // Note, compressed formats cannot be used for UAV.  We get error like:
        // ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
        // cannot be bound as an UnorderedAccessView, or cast to a format that
        // could be bound as an UnorderedAccessView.  Therefore this format 
        // does not support D3D11_BIND_UNORDERED_ACCESS.
        D3D11_TEXTURE2D_DESC blurredTexDesc;
        blurredTexDesc.Width = width;
        blurredTexDesc.Height = height;
        blurredTexDesc.MipLevels = 1;
        blurredTexDesc.ArraySize = 1;
        blurredTexDesc.Format = format;
        blurredTexDesc.SampleDesc.Count   = 1;
        blurredTexDesc.SampleDesc.Quality = 0;
        blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
        blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        blurredTexDesc.CPUAccessFlags = 0;
        blurredTexDesc.MiscFlags = 0;

        ID3D11Texture2D* blurredTex = nullptr;
        HRESULT result = device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex);
        DxErrorChecker(result);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        result = device->CreateShaderResourceView(blurredTex, &srvDesc, &mBlurredOutputTexSRV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        result = device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBlurredOutputTexUAV);

        // Views save a reference to the texture so we can release our reference.
        blurredTex->Release();
    }

    void BlurFilter::blurInPlace(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, const uint32_t blurCount)
    {
        //
        // Run the compute shader to blur the offscreen texture.
        // 
        for(size_t i = 0; i < blurCount; ++i)
        {
            //
            // Horizontal Blur Pass
            //

            // Set horizontal blur compute shader
            context->CSSetShader(Managers::ShadersManager::mHorizontalBlurCS, nullptr, 0);

            // Set shaders resources
            ID3D11ShaderResourceView* shaderResourceViews[] = { inputSRV };
            context->CSSetShaderResources(0, 1, shaderResourceViews);

            ID3D11UnorderedAccessView* unorderedAccessViews[] = { mBlurredOutputTexUAV };
            context->CSSetUnorderedAccessViews(0, 1, unorderedAccessViews, nullptr);  

            // How many groups do we need to dispatch to cover a row of pixels, where each
            // group covers 256 pixels (the 256 is defined in the ComputeShader).
            const uint32_t numGroupsX = static_cast<uint32_t> (ceilf(mWidth / 256.0f));
            context->Dispatch(numGroupsX, mHeight, 1);

            // Unbind the input texture from the CS for good housekeeping.
            ID3D11ShaderResourceView* nullSRV[] = { nullptr };
            context->CSSetShaderResources(0, 1, nullSRV);

            // Unbind output from compute shader (we are going to use this output as an input in the next pass, 
            // and a resource cannot be both an output and input at the same time.
            ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
            context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

            //
            // Vertical Blur Pass
            //

            // Set vertical blur compute shader
            context->CSSetShader(Managers::ShadersManager::mVerticalBlurCS, nullptr, 0);

            // Set shaders resources
            shaderResourceViews[0] = mBlurredOutputTexSRV;
            context->CSSetShaderResources(0, 1, shaderResourceViews);

            unorderedAccessViews[0] = inputUAV;
            context->CSSetUnorderedAccessViews(0, 1, unorderedAccessViews, nullptr);  

            // How many groups do we need to dispatch to cover a column of pixels, where each
            // group covers 256 pixels  (the 256 is defined in the ComputeShader).
            const uint32_t numGroupsY = static_cast<uint32_t> (ceilf(mHeight / 256.0f));
            context->Dispatch(mWidth, numGroupsY, 1);

            context->CSSetShaderResources(0, 1, nullSRV);
            context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
        }

        // Disable compute shader.
        context->CSSetShader(nullptr, nullptr, 0);
    }

}