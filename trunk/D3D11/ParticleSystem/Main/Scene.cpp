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
        ConstantBufferUtils::initialize(device, terrainScene.mGridHSPerFrameBuffer);
        ConstantBufferUtils::initialize(device, terrainScene.mGridDSPerFrameBuffer);
        ConstantBufferUtils::initialize(device, terrainScene.mGridVSPerObjectBuffer);

        // Terrain world matrix
        const DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&terrainScene.mWorldMatrix, translation);

        // Texture scale matrix
        const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&terrainScene.mTextureScaleMatrix, scale);
    }

    void draw(TerrainScene& terrainScene)
    {
        assert(Globals::gDirect3DData.mImmediateContext);
        ID3D11DeviceContext& context = *Globals::gDirect3DData.mImmediateContext;

        //
        // Input Assembler Stage
        //

        // Input Layout 
        ID3D11InputLayout * const inputLayout = Globals::gShaders.mTerrainIL;
        context.IASetInputLayout(inputLayout);

        // Primitive Topology
        context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);

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
        ID3D11VertexShader * const vertexShader = Globals::gShaders.mTerrainVS;
        context.VSSetShader(vertexShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&terrainScene.mWorldMatrix);
        const DirectX::XMMATRIX textureScale = DirectX::XMLoadFloat4x4(&terrainScene.mTextureScaleMatrix);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mWorld, 
                                 DirectX::XMMatrixTranspose(world));
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mTextureScale, 
                                 DirectX::XMMatrixTranspose(textureScale));
        ConstantBufferUtils::copyData(context, terrainScene.mGridVSPerObjectBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const vertexShaderBuffers[] = { terrainScene.mGridVSPerObjectBuffer.mBuffer };
        context.VSSetConstantBuffers(0, 1, vertexShaderBuffers);

        //
        // Hull Shader Stage
        //

        // Shader
        ID3D11HullShader * const hullShader = Globals::gShaders.mTerrainHS;
        context.HSSetShader(hullShader, nullptr, 0);

        // Per Frame Constant Buffer
        terrainScene.mGridHSPerFrameBuffer.mData.mEyePositionW = Globals::gCamera.mPosition;
        ConstantBufferUtils::copyData(context, terrainScene.mGridHSPerFrameBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const hullShaderBuffers = { terrainScene.mGridHSPerFrameBuffer.mBuffer };
        context.HSSetConstantBuffers(0, 1, &hullShaderBuffers);

        //
        // Domain Shader Stage
        //

        // Shader
        ID3D11DomainShader * const domainShader = Globals::gShaders.mTerrainDS;
        context.DSSetShader(domainShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(Globals::gCamera);
        const DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridDSPerFrameBuffer.mData.mWorldInverseTranspose, 
                                 DirectX::XMMatrixTranspose(worldInverseTranspose));
        DirectX::XMStoreFloat4x4(&terrainScene.mGridDSPerFrameBuffer.mData.mViewProjection,
                                 DirectX::XMMatrixTranspose(viewProjection));
        const float heightMapTexelSize = 1.0f / 512.0f;
        terrainScene.mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[0] = heightMapTexelSize;
        terrainScene.mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[1] = heightMapTexelSize;
        ConstantBufferUtils::copyData(context, terrainScene.mGridDSPerFrameBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const domainShaderBuffers = { terrainScene.mGridDSPerFrameBuffer.mBuffer };
        context.DSSetConstantBuffers(0, 1, &domainShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const domainShaderResources[] = { Globals::gShaderResources.mHeightMapSRV };
        context.DSSetShaderResources(0, 1, domainShaderResources);

        // Sampler state
        context.DSSetSamplers(0, 1, &Globals::gPipelineStates.mLinearClampSS);

        //
        // Pixel Shader Stage
        //

        // Shader
        ID3D11PixelShader* const pixelShader = Globals::gShaders.mTerrainPS;
        context.PSSetShader(pixelShader, nullptr, 0);

        // Per Frame Constant Buffer
        memcpy(
            &terrainScene.mGridPSPerFrameBuffer.mData.mDirectionalLight, 
            &terrainScene.mDirectionalLight, 
            sizeof(terrainScene.mDirectionalLight)
            );
        terrainScene.mGridPSPerFrameBuffer.mData.mEyePositionW = Globals::gCamera.mPosition;
        terrainScene.mGridPSPerFrameBuffer.mData.mMaterial = terrainScene.mTerrainMaterial;
        terrainScene.mGridPSPerFrameBuffer.mData.mTexelCellSpaceU = heightMapTexelSize;
        terrainScene.mGridPSPerFrameBuffer.mData.mTexelCellSpaceV = heightMapTexelSize;
        terrainScene.mGridPSPerFrameBuffer.mData.mWorldCellSpace = 0.5f;
        ConstantBufferUtils::copyData(context, terrainScene.mGridPSPerFrameBuffer);

        // Set constant buffers
        ID3D11Buffer * const pixelShaderBuffers[] = { terrainScene.mGridPSPerFrameBuffer.mBuffer };
        context.PSSetConstantBuffers(0, 1, pixelShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const pixelShaderResources[] = 
        { 
            Globals::gShaderResources.mHeightMapSRV,
            Globals::gShaderResources.mTerrainDiffuseMapArraySRV,
            Globals::gShaderResources.mTerrainBlendMapSRV
        };
        context.PSSetShaderResources(0, 3, pixelShaderResources);

        // Sampler state
        ID3D11SamplerState* const samplerStates[] =
        {
            Globals::gPipelineStates.mLinearClampSS,
            Globals::gPipelineStates.mLinearWrapSS
        };
        context.PSSetSamplers(0, 2, samplerStates);

        //
        // Draw
        // 
        const uint32_t baseVertexLocation = Globals::gGeometryBuffers.mBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Globals::gGeometryBuffers.mBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Globals::gGeometryBuffers.mBufferInfo->mIndexCount;
        context.DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }
}