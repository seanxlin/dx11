//////////////////////////////////////////////////////////////////////////
//
// Constant buffers for each shader
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

#include <LightHelper.h>

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
    float mHeightMapTexelWidthHeight[2];
    float mPad;
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
    float mTexelCellSpaceU;
    float mTexelCellSpaceV;
    float mWorldCellSpace;
    float mPad2;
};