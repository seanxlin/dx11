#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct ImmutableBuffer
    {
        float mFogStart;
        float mFogRange;
        float mPad1;
        float mPad2;
        DirectX::XMFLOAT4 mFogColor;
    };

    struct PerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight;
        Utils::PointLight mPointLight;
        Utils::SpotLight mSpotLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };

     struct PerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        Utils::Material mMaterial;
    };
}