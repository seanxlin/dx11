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
            {"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = device.CreateInputLayout(vertexDesc, 4, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DebugUtils::DxErrorChecker(result);
    }
}

namespace Managers
{
    ID3D11VertexShader* ShadersManager::mTerrainVS = nullptr;
    ID3D11InputLayout* ShadersManager::mTerrainIL = nullptr;
    ID3D11PixelShader* ShadersManager::mTerrainPS = nullptr;
    ID3D11HullShader* ShadersManager::mTerrainHS = nullptr;
    ID3D11DomainShader* ShadersManager::mTerrainDS = nullptr;
    
    void ShadersManager::initAll(ID3D11Device& device)
    {
        // Store shader byte code, used to create a shader.
        std::vector<char> shaderByteCode;

        // Vertex shader
        computeShaderByteCode(L"HLSL/TerrainVS.cso", shaderByteCode);
        buildShapesVertexLayout(device, shaderByteCode, mTerrainIL);
        HRESULT result = device.CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mTerrainVS);
        DebugUtils::DxErrorChecker(result);

        // Pixel shader
        computeShaderByteCode(L"HLSL/TerrainPS.cso", shaderByteCode);        
        result = device.CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mTerrainPS);
        DebugUtils::DxErrorChecker(result);

        // Hull shader
        computeShaderByteCode(L"HLSL/TerrainHS.cso", shaderByteCode);        
        result = device.CreateHullShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mTerrainHS);
        DebugUtils::DxErrorChecker(result);

        // Pixel shader
        computeShaderByteCode(L"HLSL/TerrainDS.cso", shaderByteCode);        
        result = device.CreateDomainShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mTerrainDS);
        DebugUtils::DxErrorChecker(result);
    }
    
    void ShadersManager::destroyAll()
    {
        mTerrainVS->Release();
        mTerrainIL->Release();
        mTerrainPS->Release();
        mTerrainHS->Release();
        mTerrainDS->Release();
    }
}
