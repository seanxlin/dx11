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

    void buildBezierSurfaceVertexLayout(ID3D11Device * const device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
    {
        assert(device);
        assert(!inputLayout);
        assert(!shaderByteCode.empty());

        // Create the vertex input layout for bezier surface
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = device->CreateInputLayout(vertexDesc, 1, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DebugUtils::DxErrorChecker(result);
    }
}

namespace Managers
{
    ID3D11VertexShader* ShadersManager::mBezierSurfaceVS = nullptr;
    ID3D11InputLayout* ShadersManager::mBezierSurfaceIL = nullptr;        
    ID3D11PixelShader* ShadersManager::mBezierSurfacePS = nullptr;
    ID3D11HullShader* ShadersManager::mBezierSurfaceHS = nullptr;
    ID3D11DomainShader* ShadersManager::mBezierSurfaceDS = nullptr;
    
    void ShadersManager::initAll(ID3D11Device * const device)
    {
        assert(device);

        // Store shader byte code, used to create a shader.
        std::vector<char> shaderByteCode;

        // Vertex shader
        computeShaderByteCode(L"HLSL/BezierSurfaceVS.cso", shaderByteCode);
        buildBezierSurfaceVertexLayout(device, shaderByteCode, mBezierSurfaceIL);
        HRESULT result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mBezierSurfaceVS);
        DebugUtils::DxErrorChecker(result);

        // Pixel shader
        computeShaderByteCode(L"HLSL/BezierSurfacePS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mBezierSurfacePS);
        DebugUtils::DxErrorChecker(result);

        // Hull shader
        computeShaderByteCode(L"HLSL/BezierSurfaceHS.cso", shaderByteCode);        
        result = device->CreateHullShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mBezierSurfaceHS);
        DebugUtils::DxErrorChecker(result);

        // Domain Shader
        computeShaderByteCode(L"HLSL/BezierSurfaceDS.cso", shaderByteCode);        
        result = device->CreateDomainShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mBezierSurfaceDS);
        DebugUtils::DxErrorChecker(result);
    }
    
    void ShadersManager::destroyAll()
    {
        mBezierSurfaceVS->Release();
        mBezierSurfaceIL->Release();      
        mBezierSurfacePS->Release();
        mBezierSurfaceHS->Release();
        mBezierSurfaceDS->Release();
    }
}
