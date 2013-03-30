#pragma once

#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

namespace Utils
{
    struct BoundingSphere
    {
        BoundingSphere() 
            : mCenter(0.0f, 0.0f, 0.0f)
            , mRadius(0.0f) 
        {

        }

        DirectX::XMFLOAT3 mCenter;
        float mRadius;
    };

    // Helper class for implementing shadows via shadow mapping algorithm.
    class ShadowMapper
    {
    public:
        ShadowMapper(ID3D11Device& device, const uint32_t width, const uint32_t height);
        ~ShadowMapper();

        ID3D11ShaderResourceView& depthMapSRV();

        void bindDepthStencilViewAndSetNullRenderTarget(ID3D11DeviceContext& deviceContext);

    private:
        ShadowMapper(const ShadowMapper& shadowMap);
        ShadowMapper& operator=(const ShadowMapper& rhs);

    private:
        uint32_t mWidth;
        uint32_t mHeight;

        ID3D11ShaderResourceView* mDepthMapSRV;
        ID3D11DepthStencilView* mDepthMapDSV;

        D3D11_VIEWPORT mViewport;
    };
}