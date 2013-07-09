#include "Scene.h"

#include <MathHelper.h>

#include "Globals.h"

namespace TerrainSceneUtils
{
    void init(TerrainScene& terrainScene)
    {
        // Directional lights.
        terrainScene.mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
        terrainScene.mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
        terrainScene.mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, -0.707f, 0.0f);

        terrainScene.mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        terrainScene.mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        terrainScene.mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        terrainScene.mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

        // Initialize material
        terrainScene.mTerrainMaterial.mAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mTerrainMaterial.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mTerrainMaterial.mSpecular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
        terrainScene.mTerrainMaterial.mReflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

        // Initialize constant buffers
        assert(Globals::gDirect3DData.mDevice);
        ID3D11Device& device = *Globals::gDirect3DData.mDevice;
        ConstantBufferUtils::initialize(device, terrainScene.mGridPSPerFrameBuffer);
        ConstantBufferUtils::initialize(device, terrainScene.mGridVSPerObjectBuffer);

        // World matrix
        const DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&terrainScene.mWorldMatrix, translation);

        // Texture scale matrix
        const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&terrainScene.mTextureScaleMatrix, scale);

        //
        // Compute Shader
        //

        // Shader
        assert(Globals::gDirect3DData.mImmediateContext);
        ID3D11DeviceContext& context = *Globals::gDirect3DData.mImmediateContext;
        context.CSSetShader(Globals::gShaders.mCS, nullptr, 0);

        // Set shaders resources
        const uint32_t srvMaxSlots = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
        ID3D11ShaderResourceView* shaderResourceViews[srvMaxSlots] = 
        { 
            Globals::gShaderResources.mDiffuseMapSRV 
        };
        context.CSSetShaderResources(0, 
                                     srvMaxSlots, 
                                     shaderResourceViews);

        const uint32_t uavMaxSlots = D3D11_PS_CS_UAV_REGISTER_COUNT;
        ID3D11UnorderedAccessView* unorderedAccessViews[uavMaxSlots] = 
        { 
            Globals::gShaderResources.mCSResultsUAV
        };
        context.CSSetUnorderedAccessViews(0, 
                                          uavMaxSlots, 
                                          unorderedAccessViews, 
                                          nullptr); 
        
        // For a 512 x 512 texture we will consider 32 x 32 thread groups
        // with 16 x 16 threads per group
        context.Dispatch(512, 512, 1);

        memset(shaderResourceViews, 0, sizeof(ID3D11ShaderResourceView*) * srvMaxSlots);
        context.CSSetShaderResources(0, 
                                     srvMaxSlots, 
                                     shaderResourceViews);

        memset(unorderedAccessViews, 0, sizeof(ID3D11ShaderResourceView*) * uavMaxSlots);
        context.CSSetUnorderedAccessViews(0, 
                                          uavMaxSlots, 
                                          unorderedAccessViews,
                                          nullptr);
    }

    void draw(TerrainScene& terrainScene)
    {
        assert(Globals::gDirect3DData.mImmediateContext);
        ID3D11DeviceContext& context = *Globals::gDirect3DData.mImmediateContext;

        //
        // Input Assembler Stage
        //

        // Input Layout 
        ID3D11InputLayout * const inputLayout = Globals::gShaders.mIL;
        context.IASetInputLayout(inputLayout);

        // Primitive Topology
        context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Vertex Buffer
        ID3D11Buffer* vertexBuffer = Globals::gGeometryBuffers.mBufferInfo->mVertexBuffer;
        uint32_t stride = sizeof(Vertex);
        uint32_t offset = 0;
        context.IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // Index Buffer
        ID3D11Buffer* indexBuffer = Globals::gGeometryBuffers.mBufferInfo->mIndexBuffer;
        context.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Vertex Shader Stage
        //

        // Shader
        ID3D11VertexShader * const vertexShader = Globals::gShaders.mVS;
        context.VSSetShader(vertexShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&terrainScene.mWorldMatrix);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mWorld, 
                                 DirectX::XMMatrixTranspose(world));

        const DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mWorldInverseTranspose, 
                                 DirectX::XMMatrixTranspose(worldInverseTranspose));

        const DirectX::XMMATRIX textureScale = DirectX::XMLoadFloat4x4(&terrainScene.mTextureScaleMatrix);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mTextureScale, 
                                 DirectX::XMMatrixTranspose(textureScale));

        ConstantBufferUtils::applyChanges(context, terrainScene.mGridVSPerObjectBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const vertexShaderBuffers[] = 
        { 
            terrainScene.mGridVSPerObjectBuffer.mBuffer
        };
        context.VSSetConstantBuffers(0, 1, vertexShaderBuffers);        

        //
        // Pixel Shader Stage
        //

        // Shader
        ID3D11PixelShader* const pixelShader = Globals::gShaders.mPS;
        context.PSSetShader(pixelShader, nullptr, 0);

        // Per Frame Constant Buffer
        memcpy(
            &terrainScene.mGridPSPerFrameBuffer.mData.mDirectionalLight, 
            &terrainScene.mDirectionalLight, 
            sizeof(terrainScene.mDirectionalLight)
            );
        terrainScene.mGridPSPerFrameBuffer.mData.mEyePositionW = Globals::gCamera.mPosition;
        terrainScene.mGridPSPerFrameBuffer.mData.mMaterial = terrainScene.mTerrainMaterial;

        ConstantBufferUtils::applyChanges(context, terrainScene.mGridPSPerFrameBuffer);

        // Set constant buffers
        ID3D11Buffer * const pixelShaderBuffers[] = 
        { 
            terrainScene.mGridPSPerFrameBuffer.mBuffer 
        };
        context.PSSetConstantBuffers(0, 1, pixelShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const pixelShaderResources[] = 
        { 
            Globals::gShaderResources.mCSResultsSRV
        };
        context.PSSetShaderResources(0, 1, pixelShaderResources);

        // Sampler state
        ID3D11SamplerState* const samplerStates[] =
        {
            Globals::gPipelineStates.mLinearClampSS,
        };
        context.PSSetSamplers(0, 1, samplerStates);

        //
        // Draw
        // 
        const uint32_t baseVertexLocation = Globals::gGeometryBuffers.mBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Globals::gGeometryBuffers.mBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Globals::gGeometryBuffers.mBufferInfo->mIndexCount;
        context.DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }
}