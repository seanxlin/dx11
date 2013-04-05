#pragma once

#include <cassert>
#include <cstdint>
#include <d3d11.h>

template<typename T>
struct ConstantBuffer
{
    ConstantBuffer()
        : mBuffer(nullptr)
    {

    }

    // Structure instance mirroring the data stored in
    // the constant buffer.
    T mData;

    ID3D11Buffer* mBuffer;
};

namespace ConstantBufferUtils
{
    template<typename T>
    HRESULT initialize(ID3D11Device& device, ConstantBuffer<T>& constantBuffer)
    {
        assert(constantBuffer.mBuffer == nullptr);

        // Make constant buffer multiple of 16 bytes.
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;

        // To ensure 16 byte alignment
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(T) + (16 - (sizeof(T) % 16)));
        bufferDesc.StructureByteStride = 0;

        return device.CreateBuffer(&bufferDesc, 0, &constantBuffer.mBuffer);
    }

    // Copies the system memory constant buffer data to the GPU
    // constant buffer. This call should be made as infrequently
    // as possible.
    template<typename T>
    void applyChanges(ID3D11DeviceContext& deviceContext, ConstantBuffer<T>& constantBuffer)
    {
        assert(constantBuffer.mBuffer);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        deviceContext.Map(constantBuffer.mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        CopyMemory(mappedResource.pData, &constantBuffer.mData, sizeof(T));
        deviceContext.Unmap(constantBuffer.mBuffer, 0);
    }
}
