#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

namespace Shaders
{
    //
    // Vertex Shader
    //
    struct GridVSPerFrameBuffer
    {
        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mTextureScale;
    };

    //
    // Hull Shader
    //
    struct GridHSPerFrameBuffer
    {
        DirectX::XMFLOAT3 mEyePositionW;
    };

    //
    // Domain Shader
    //
    struct GridDSPerFrameBuffer
    {
        DirectX::XMFLOAT4X4 mWorldInverseTranspose;
        DirectX::XMFLOAT4X4 mViewProjection;
        float mHeightMapWidthHeight[2];
        float mPad;
    };

    //
    // Pixel shader
    //
    struct GridPSPerFrameBuffer
    {
        Utils::DirectionalLight mDirectionalLight[3];
        DirectX::XMFLOAT3 mEyePositionW;
        float mPad;
        Utils::Material mMaterial;
    };
}