#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct FloorVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        DirectX::XMFLOAT4X4 mShadowTransform;
    };

    struct ShapesVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        DirectX::XMFLOAT4X4 mShadowTransform;
    };

    struct CommonPSPerFrameBuffer
    {
        DirectionalLight mDirectionalLight[3];
        DirectX::XMFLOAT3 mEyePositionW;
    };

    struct CommonPSPerObjectBuffer
    {
        Material mMaterial;
    };

    struct ShadowMapVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mLightViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
    };

    struct FloorShadowMapVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mLightViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
    };
}