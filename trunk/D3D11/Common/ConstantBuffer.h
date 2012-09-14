#pragma once

#include <cassert>
#include <cstdint>
#include <D3DX11.h>

namespace Shaders
{
    template<typename T>
    class ConstantBuffer
    {
    public:
        ConstantBuffer() 
            : mBuffer(nullptr)
            , mInitialized(false)
            
        {
        }
        ~ConstantBuffer()
        {
            mBuffer->Release();
        }
       
        // Public structure instance mirroring the data stored in
        // the constant buffer.
        T mData;

        __forceinline ID3D11Buffer* buffer() const { return mBuffer; }

        HRESULT initialize(ID3D11Device * const device)
        {
            // Make constant buffer multiple of 16 bytes.
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags = 0;
            bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(T) + (16 - (sizeof(T) % 16)));
            bufferDesc.StructureByteStride = 0;
            const HRESULT result = device->CreateBuffer(&bufferDesc, 0, &mBuffer);
            mInitialized = true;

            return result;
        }
        
        // Copies the system memory constant buffer data to the GPU
        // constant buffer. This call should be made as infrequently
        // as possible.        
        void applyChanges(ID3D11DeviceContext * const deviceContext)
        {
            assert(mInitialized && L"ConstantBuffer not initialized.");

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            deviceContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            CopyMemory(mappedResource.pData, &mData, sizeof(T));
            deviceContext->Unmap(mBuffer, 0);
        }
    private:
        ConstantBuffer(const ConstantBuffer<T>& rhs);
        ConstantBuffer<T>& operator=(const ConstantBuffer<T>& rhs);

        ID3D11Buffer* mBuffer;
        bool mInitialized;
    };
}