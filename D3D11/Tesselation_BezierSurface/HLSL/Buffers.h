#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    struct BezierSurfaceDSPerFrameBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mWorldViewProjection;
        DirectX::XMFLOAT4X4 mTexTransform;
        float mInGameTime;
    };

    struct BezierSurfaceHSPerFrameBuffer
    {
        float mTesselationFactor;
    };

    struct BezierSurfacePSPerFrameBuffer
    {
        DirectionalLight mDirectionalLight;
        Material mMaterial;
        DirectX::XMFLOAT3 mEyePositionW;
    };
}