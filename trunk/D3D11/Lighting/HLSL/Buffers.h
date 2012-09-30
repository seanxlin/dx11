#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
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
        Utils::Material mMaterial;
    };
}