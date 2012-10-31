#pragma once

#include <DirectXMath.h>

namespace Geometry
{
    struct LandVertex
    {
        DirectX::XMFLOAT3 mPosition;
        DirectX::XMFLOAT3 mNormal;
        DirectX::XMFLOAT2 mTexCoord;
    };

    struct BillboardVertex
    {
        DirectX::XMFLOAT3 mPosition;
        DirectX::XMFLOAT2 mSize;
        float mPad1;
        float mPad2;
    };
}