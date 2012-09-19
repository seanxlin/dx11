#pragma once

#include <DirectXMath.h>

namespace Shaders
{
    struct PerObjectBuffer
    {
        DirectX::XMFLOAT4X4 mWorldViewProjectionTranspose;
    };
}