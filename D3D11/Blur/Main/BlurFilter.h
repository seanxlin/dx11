#pragma once

#include <cstdint>
#include <D3D11.h>

struct ID3D11Device;

namespace Framework
{
    class BlurFilter
    {
    public:
        inline BlurFilter();
        inline ~BlurFilter();

        ID3D11ShaderResourceView* blurredOutput();

        // The width and height should match the dimensions of the input texture to blur.
        // It is OK to call init() again to reinitialize the blur filter with a different 
        // dimension or format.
        void init(ID3D11Device* device, const uint32_t width, const uint32_t height, DXGI_FORMAT format);

        // Blurs the input texture blurCount times.  Note that this modifies the input texture, not a copy of it.
        void blurInPlace(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, const uint32_t blurCount);

    private:
        uint32_t mWidth;
        uint32_t mHeight;
        DXGI_FORMAT mFormat;

        ID3D11ShaderResourceView* mBlurredOutputTexSRV;
        ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
    };

    inline BlurFilter::BlurFilter()
        : mWidth(0)
        , mHeight(0)
        , mBlurredOutputTexSRV(nullptr)
        , mBlurredOutputTexUAV(nullptr)
    {

    }

    inline BlurFilter::~BlurFilter()
    {
        mBlurredOutputTexSRV->Release();
        mBlurredOutputTexUAV->Release();
    }
}