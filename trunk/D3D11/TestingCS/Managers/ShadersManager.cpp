#include "ShadersManager.h"

#include <cassert>
#include <D3D11.h>
#include <fstream>
#include <string>
#include <vector>

#include <DxErrorChecker.h>

namespace 
{
    void computeShaderByteCode(const std::wstring& fileName, std::vector<char>& shaderByteCode)
    {
        assert(!fileName.empty());

        // Clear vector content
        shaderByteCode.clear();

        // Open shader file, compile it and 
        std::ifstream fin;
        fin.open(fileName.c_str(), std::ios::binary); 

        assert(fin.is_open());

        fin.seekg(0, std::ios_base::end); 
        size_t size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        shaderByteCode.resize(size); 
        fin.read(&shaderByteCode[0], size); 
        fin.close();
    }

    void buildShapesVertexLayout(ID3D11Device& device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
    {
        assert(!inputLayout);
        assert(!shaderByteCode.empty());

        // Create the vertex input layout for land and screen quad
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = device.CreateInputLayout(vertexDesc, 3, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DxErrorChecker(result);
    }
}

namespace ShadersUtils
{    
    void initAll(ID3D11Device& device, Shaders& shaders)
    {
        assert(shaders.mVS == nullptr);
        assert(shaders.mIL == nullptr);
        assert(shaders.mPS == nullptr);
        assert(shaders.mCS == nullptr);

        // Store shader byte code, used to create a shader.
        std::vector<char> shaderByteCode;

        // Vertex shader
        computeShaderByteCode(L"HLSL/VS.cso", shaderByteCode);
        buildShapesVertexLayout(device, shaderByteCode, shaders.mIL);
        HRESULT result = device.CreateVertexShader(
            &shaderByteCode[0],
            shaderByteCode.size(),
            nullptr,
            &shaders.mVS);
        DxErrorChecker(result);

        // Pixel shader
        computeShaderByteCode(L"HLSL/PS.cso", shaderByteCode);        
        result = device.CreatePixelShader(
            &shaderByteCode[0], 
            shaderByteCode.size(), 
            nullptr, 
            &shaders.mPS);
        DxErrorChecker(result);

        // Compute Shader
        computeShaderByteCode(L"HLSL/CS.cso", shaderByteCode);        
        result = device.CreateComputeShader(
            &shaderByteCode[0], 
            shaderByteCode.size(), 
            nullptr, 
            &shaders.mCS);
        DxErrorChecker(result);
    }
    
    void destroyAll(Shaders& shaders)
    {
        assert(shaders.mVS);
        assert(shaders.mIL);
        assert(shaders.mPS);
        assert(shaders.mCS);

        shaders.mVS->Release();
        shaders.mIL->Release();
        shaders.mPS->Release();
        shaders.mCS->Release();
    }
}
