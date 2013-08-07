//////////////////////////////////////////////////////////////////////////
//
// Constant buffer class used by shaders
//
//////////////////////////////////////////////////////////////////////////

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

        // If the bind flag is D3D11_BIND_CONSTANT_BUFFER, 
        // you must set the ByteWidth value in multiples of 16, 
        // and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
        const size_t baseAlignment = 16;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = 
            static_cast<uint32_t>(sizeof(T) + (baseAlignment - (sizeof(T) % baseAlignment)));
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;        
        bufferDesc.StructureByteStride = 0;

        return device.CreateBuffer(&bufferDesc, 0, &constantBuffer.mBuffer);
    }

    // Copies the content from the constant buffer to the buffer 
    // that will be used by the GPU.
    // This call should be made as infrequently as possible.
    template<typename T>
    void copyData(ID3D11DeviceContext& deviceContext, ConstantBuffer<T>& constantBuffer)
    {
        assert(constantBuffer.mBuffer);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        deviceContext.Map(constantBuffer.mBuffer, 
                          0, 
                          D3D11_MAP_WRITE_DISCARD, 
                          0, 
                          &mappedResource);
        CopyMemory(mappedResource.pData, &constantBuffer.mData, sizeof(T));
        deviceContext.Unmap(constantBuffer.mBuffer, 0);
    }
}
