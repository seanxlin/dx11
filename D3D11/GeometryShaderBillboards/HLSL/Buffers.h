#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct CommonPerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight;
        Utils::SpotLight mSpotLight;
        DirectX::XMFLOAT3 mEyePositionW;
    };

    struct LandPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        Utils::Material mMaterial;
    };

     struct BillboardsPerObjectBuffer
     {
         DirectX::XMFLOAT4X4 mViewProjection;
         Utils::Material mMaterial;
     };
}