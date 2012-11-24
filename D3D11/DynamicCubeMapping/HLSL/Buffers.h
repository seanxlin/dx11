#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct LandPerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight;
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
        Utils::Material mMaterial;
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
        Utils::Material mMaterial;
    };

    struct SpherePSPerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };
}