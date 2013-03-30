#pragma once

#include <cassert>
#include <cstdint>
#include <d3d11.h>

namespace Shaders
{
    template<typename T>
    class ConstantBuffer
    {
    public:
        inline ConstantBuffer();
        inline ~ConstantBuffer();
       
        // Public structure instance mirroring the data stored in
        // the constant buffer.
        T mData;

        inline ID3D11Buffer& buffer() const;

        HRESULT ConstantBuffer::initialize(ID3D11Device& device)
        {
            // Make constant buffer multiple of 16 bytes.
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags = 0;

            // To ensure 16 byte alignment
            bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(T) + (16 - (sizeof(T) % 16)));
            bufferDesc.StructureByteStride = 0;

            return device.CreateBuffer(&bufferDesc, 0, &mBuffer);
        }

        // Copies the system memory constant buffer data to the GPU
        // constant buffer. This call should be made as infrequently
        // as possible.        
        void ConstantBuffer::applyChanges(ID3D11DeviceContext& deviceContext)
        {
            assert(mBuffer);

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            deviceContext.Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            CopyMemory(mappedResource.pData, &mData, sizeof(T));
            deviceContext.Unmap(mBuffer, 0);
        }

    private:
        ConstantBuffer(const ConstantBuffer<T>& rhs);
        ConstantBuffer<T>& operator=(const ConstantBuffer<T>& rhs);

        ID3D11Buffer* mBuffer;
    };

    template<typename T>
    inline ConstantBuffer<T>::ConstantBuffer() 
        : mBuffer(nullptr)
    {

    }

    template<typename T>
    inline ConstantBuffer<T>::~ConstantBuffer()
    {
        mBuffer->Release();
    }

    template<typename T>
    inline ID3D11Buffer& ConstantBuffer<T>::buffer() const 
    {
        assert(mBuffer);

        return *mBuffer; 
    }
}