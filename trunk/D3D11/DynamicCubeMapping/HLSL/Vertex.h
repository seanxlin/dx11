#pragma once

#include <DirectXMath.h>

namespace Geometry
{
    struct CommonVertex
    {
        DirectX::XMFLOAT3 mPosition;
        DirectX::XMFLOAT3 mNormal;
        DirectX::XMFLOAT2 mTexCoord;
    };

    struct SkyVertex
    {
        DirectX::XMFLOAT3 mPosition;
    };
}