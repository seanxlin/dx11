#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct PerFrameBuffer
    {
        DirectionalLight mDirectionalLight;
        PointLight mPointLight;
        SpotLight mSpotLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };

     struct PerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        Material mMaterial;
    };
}