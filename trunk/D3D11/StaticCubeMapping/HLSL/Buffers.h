#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct LandPerFrameBuffer
    {
        DirectionalLight mDirectionalLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };

    struct LandVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
    };

    struct LandPSPerObjectBuffer
    {
        Material mMaterial;
    };

    struct SkyPerFrameBuffer
    {
        DirectX::XMFLOAT4X4 mWorldViewProjection;
    };

    struct SphereVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
    };

    struct SpherePSPerObjectBuffer
    {
        Material mMaterial;
    };

    struct SpherePSPerFrameBuffer
    {
        DirectionalLight mDirectionalLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };
}