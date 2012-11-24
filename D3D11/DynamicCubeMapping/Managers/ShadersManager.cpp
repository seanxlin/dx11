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

    void buildLandVertexLayout(ID3D11Device * const device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
    {
        assert(device);
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
        const HRESULT result = device->CreateInputLayout(vertexDesc, 3, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DebugUtils::DxErrorChecker(result);
    }

    void buildSphereVertexLayout(ID3D11Device * const device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
    {
        assert(device);
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
        const HRESULT result = device->CreateInputLayout(vertexDesc, 3, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DebugUtils::DxErrorChecker(result);
    }

    void buildSkyVertexLayout(ID3D11Device * const device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
    {
        assert(device);
        assert(!inputLayout);
        assert(!shaderByteCode.empty());

        // Create the vertex input layout for land and screen quad
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        // Create the input layout
        const HRESULT result = device->CreateInputLayout(vertexDesc, 1, &shaderByteCode[0], 
            shaderByteCode.size(), &inputLayout);

        DebugUtils::DxErrorChecker(result);
    }
}

namespace Managers
{
    ID3D11VertexShader* ShadersManager::mLandVS = nullptr;
    ID3D11InputLayout* ShadersManager::mLandIL = nullptr;        
    ID3D11PixelShader* ShadersManager::mLandPS = nullptr;

    ID3D11VertexShader* ShadersManager::mSphereVS = nullptr;
    ID3D11InputLayout* ShadersManager::mSphereIL = nullptr;
    ID3D11PixelShader* ShadersManager::mSpherePS = nullptr;

    ID3D11VertexShader* ShadersManager::mSkyVS = nullptr;
    ID3D11InputLayout* ShadersManager::mSkyIL = nullptr;
    ID3D11PixelShader* ShadersManager::mSkyPS = nullptr;
    
    void ShadersManager::initAll(ID3D11Device * const device)
    {
        assert(device);

        // Store shader byte code, used to create a shader.
        std::vector<char> shaderByteCode;

        //
        // Vertex shaders
        //
        computeShaderByteCode(L"HLSL/LandVS.cso", shaderByteCode);
        buildLandVertexLayout(device, shaderByteCode, mLandIL);
        HRESULT result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mLandVS);
        DebugUtils::DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/SphereVS.cso", shaderByteCode);
        buildSphereVertexLayout(device, shaderByteCode, mSphereIL);
        result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mSphereVS);
        DebugUtils::DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/SkyVS.cso", shaderByteCode);
        buildSkyVertexLayout(device, shaderByteCode, mSkyIL);
        result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mSkyVS);
        DebugUtils::DxErrorChecker(result);

        //
        // Pixel shaders
        //
        computeShaderByteCode(L"HLSL/LandPS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mLandPS);
        DebugUtils::DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/SpherePS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mSpherePS);
        DebugUtils::DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/SkyPS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mSkyPS);
        DebugUtils::DxErrorChecker(result);
    }
    
    void ShadersManager::destroyAll()
    {
        mLandVS->Release();
        mLandIL->Release();      
        mLandPS->Release();

        mSphereVS->Release();
        mSphereIL->Release();
        mSpherePS->Release();

        mSkyVS->Release();
        mSkyIL->Release();
        mSkyPS->Release();
    }
}
