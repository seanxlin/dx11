#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

//
// Vertex Shader
//
struct GridVSPerFrameBuffer
{
    DirectX::XMFLOAT4X4 mWorld;
    DirectX::XMFLOAT4X4 mViewProjection;
    DirectX::XMFLOAT4X4 mWorldInverseTranspose;
    DirectX::XMFLOAT4X4 mTextureScale;
};

//
// Pixel shader
//
struct GridPSPerFrameBuffer
{
    DirectionalLight mDirectionalLight[3];
    DirectX::XMFLOAT3 mEyePositionW;
    float mPad1;
    Material mMaterial;
};