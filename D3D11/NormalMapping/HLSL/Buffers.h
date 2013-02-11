#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct ShapesVSPerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
    };

    struct ShapesPSPerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight[3];
        DirectX::XMFLOAT3 mEyePositionW;
    };

    struct ShapesPSPerObjectBuffer
    {
        Utils::Material mMaterial;
    };
}