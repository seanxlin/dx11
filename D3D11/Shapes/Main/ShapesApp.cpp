#include "ShapesApp.h"
 
#include <fstream>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <vector>

#include "HLSL/Vertex.h"

#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace Framework
{
    void ShapesApp::drawScene()
    {
        // Update state
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::White));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->IASetInputLayout(mInputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const uint32_t stride = sizeof(Geometry::Vertex);
        const uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* constantBuffer = mPerObjectBuffer.mBuffer;
        mImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
        mImmediateContext->RSSetState(mWireframeRS);

        // Compute view * projection matrix
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&mProjection);
        const DirectX::XMMATRIX viewProjection = view * projection;

        // Draw the grid, updating first per object buffer
        DirectX::XMMATRIX worldViewProjection = DirectX::XMLoadFloat4x4(&mGridWorld) * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));

        ConstantBufferUtils::copyData(*mImmediateContext, mPerObjectBuffer);
        mImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

        // Draw the box, updating first per object buffer
        worldViewProjection = DirectX::XMLoadFloat4x4(&mBoxWorld) * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));
        
        ConstantBufferUtils::copyData(*mImmediateContext, mPerObjectBuffer);
        mImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        // Draw center sphere, updating first per object buffer
        worldViewProjection = DirectX::XMLoadFloat4x4(&mCenterSphere) * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));
        
       ConstantBufferUtils::copyData(*mImmediateContext,  mPerObjectBuffer);
        mImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

        // Draw the cylinders, updating first per object buffer
        for(int i = 0; i < 10; ++i)
        {
            worldViewProjection = DirectX::XMLoadFloat4x4(&mCylWorld[i]) * viewProjection;
            DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
                DirectX::XMMatrixTranspose(worldViewProjection));
            
            ConstantBufferUtils::copyData(*mImmediateContext, mPerObjectBuffer);
            mImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }

        // Draw the spheres, updating first per object buffer
        for(int i = 0; i < 10; ++i)
        {
            worldViewProjection = XMLoadFloat4x4(&mSphereWorld[i]) * viewProjection;
            DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
                DirectX::XMMatrixTranspose(worldViewProjection));

            ConstantBufferUtils::copyData(*mImmediateContext, mPerObjectBuffer);
            mImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }

        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void ShapesApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

            // Update angles based on input to orbit camera around box.
            mTheta += dx;
            mPhi += dy;

            // Restrict the angle mPhi.
            mPhi = MathHelper::clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
        }
        else if( (btnState & MK_RBUTTON) != 0 )
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            const float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
            const float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

            // Update the camera radius based on input.
            mRadius += dx - dy;

            // Restrict the radius.
            mRadius = MathHelper::clamp(mRadius, 3.0f, 200.0f);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void ShapesApp::buildGeometryBuffers()
    {
        MeshData box;
        MeshData grid;
        MeshData sphere;
        MeshData cylinder;

        GeometryGenerator::generateBox(1.0f, 1.0f, 1.0f, box);
        GeometryGenerator::generateGrid(20.0f, 30.0f, 60, 40, grid);
        GeometryGenerator::generateSphere(0.5f, 20, 20, sphere);
        GeometryGenerator::generateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

        // Cache the vertex offsets to each object in the concatenated vertex buffer.
        mBoxVertexOffset = 0;
        mGridVertexOffset = static_cast<uint32_t> (box.mVertices.size());
        mSphereVertexOffset = mGridVertexOffset + static_cast<uint32_t> (grid.mVertices.size());
        mCylinderVertexOffset = mSphereVertexOffset + static_cast<uint32_t> (sphere.mVertices.size());

        // Cache the index count of each object.
        mBoxIndexCount = static_cast<uint32_t> (box.mIndices.size());
        mGridIndexCount = static_cast<uint32_t> (grid.mIndices.size());
        mSphereIndexCount = static_cast<uint32_t> (sphere.mIndices.size());
        mCylinderIndexCount = static_cast<uint32_t> (cylinder.mIndices.size());

        // Cache the starting index for each object in the concatenated index buffer.
        mBoxIndexOffset = 0;
        mGridIndexOffset = mBoxIndexCount;
        mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
        mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

        const uint64_t totalVertexCount = box.mVertices.size() + grid.mVertices.size() 
            + sphere.mVertices.size() + cylinder.mVertices.size();

        const uint64_t totalIndexCount = mBoxIndexCount + mGridIndexCount 
            + mSphereIndexCount + mCylinderIndexCount;

        // Extract the vertex elements we are interested in and pack the
        // vertices of all the meshes into one vertex buffer.
        std::vector<Geometry::Vertex> vertices(totalVertexCount);
        DirectX::XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

        uint32_t k = 0;
        for(size_t i = 0; i < box.mVertices.size(); ++i, ++k)
        {
            vertices[k].mPosition = box.mVertices[i].mPosition;
            vertices[k].mColor = black;
        }

        for(size_t i = 0; i < grid.mVertices.size(); ++i, ++k)
        {
            vertices[k].mPosition   = grid.mVertices[i].mPosition;
            vertices[k].mColor = black;
        }

        for(size_t i = 0; i < sphere.mVertices.size(); ++i, ++k)
        {
            vertices[k].mPosition = sphere.mVertices[i].mPosition;
            vertices[k].mColor = black;
        }

        for(size_t i = 0; i < cylinder.mVertices.size(); ++i, ++k)
        {
            vertices[k].mPosition = cylinder.mVertices[i].mPosition;
            vertices[k].mColor = black;
        }

        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::Vertex) * totalVertexCount);
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];
        mDevice->CreateBuffer(&vertexBufferDesc, &initData, &mVertexBuffer);

        // Pack the indices of all the meshes into one index buffer.
        std::vector<uint32_t> indices;
        indices.insert(indices.end(), box.mIndices.begin(), box.mIndices.end());
        indices.insert(indices.end(), grid.mIndices.begin(), grid.mIndices.end());
        indices.insert(indices.end(), sphere.mIndices.begin(), sphere.mIndices.end());
        indices.insert(indices.end(), cylinder.mIndices.begin(), cylinder.mIndices.end());

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(uint32_t) * totalIndexCount);
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &indices[0];
        mDevice->CreateBuffer(&indexBufferDesc, &initData, &mIndexBuffer);
    }

    void ShapesApp::buildShaders()
    {
        //
        // Read compiled vertex and pixel shaders and create them.
        //
        std::ifstream fin;
        fin.open("HLSL/VertexShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size_t size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); ;
        std::vector<char> compiledShader(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();

        buildVertexLayout(compiledShader);

        HRESULT result = mDevice->CreateVertexShader(&compiledShader[0], size, nullptr, &mVertexShader);
        DxErrorChecker(result);
        
        fin.open("HLSL/PixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mPixelShader);
        DxErrorChecker(result);

        // Bind shaders to the rendering pipeline
        mImmediateContext->VSSetShader(mVertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(mPixelShader, nullptr, 0);
    }
}