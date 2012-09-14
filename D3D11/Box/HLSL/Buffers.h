#pragma once

#include <DirectXMath.h>

namespace Shaders
{
    struct PerFrameBuffer
    {
        DirectX::XMFLOAT4X4 mWorldViewProjectionTranspose;
    };
}