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

    void buildCommonVertexLayout(ID3D11Device * const device, std::vector<char>& shaderByteCode, ID3D11InputLayout* &inputLayout)
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

        DxErrorChecker(result);
    }
}

namespace Managers
{
    ID3D11VertexShader* ShadersManager::mLandVS = nullptr;
    ID3D11InputLayout* ShadersManager::mCommonIL = nullptr;        
    ID3D11PixelShader* ShadersManager::mLandPS = nullptr;

    ID3D11VertexShader* ShadersManager::mScreenQuadVS = nullptr;
    ID3D11InputLayout* ShadersManager::mScreenQuadIL = nullptr;
    ID3D11PixelShader* ShadersManager::mScreenQuadPS = nullptr;

    ID3D11ComputeShader* ShadersManager::mHorizontalBlurCS = nullptr;
    ID3D11ComputeShader* ShadersManager::mVerticalBlurCS = nullptr;
    
    void ShadersManager::initAll(ID3D11Device * const device)
    {
        assert(device);

        // Store shader byte code, used to create a shader.
        std::vector<char> shaderByteCode;

        //
        // Vertex shaders
        //
        computeShaderByteCode(L"HLSL/LandVS.cso", shaderByteCode);
        buildCommonVertexLayout(device, shaderByteCode, mCommonIL);
        HRESULT result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mLandVS);
        DxErrorChecker(result);
        
        computeShaderByteCode(L"HLSL/ScreenQuadVS.cso", shaderByteCode);
        buildCommonVertexLayout(device, shaderByteCode, mScreenQuadIL);
        result = device->CreateVertexShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mScreenQuadVS);
        DxErrorChecker(result);

        //
        // Pixel shaders
        //
        computeShaderByteCode(L"HLSL/LandPS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mLandPS);
        DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/ScreenQuadPS.cso", shaderByteCode);        
        result = device->CreatePixelShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mScreenQuadPS);
        DxErrorChecker(result);

        //
        // Compute shaders
        //
        computeShaderByteCode(L"HLSL/HorizontalBlurCS.cso", shaderByteCode);        
        result = device->CreateComputeShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mHorizontalBlurCS);
        DxErrorChecker(result);

        computeShaderByteCode(L"HLSL/VerticalBlurCS.cso", shaderByteCode);        
        result = device->CreateComputeShader(&shaderByteCode[0], shaderByteCode.size(), nullptr, &mVerticalBlurCS);
        DxErrorChecker(result);
    }
    
    void ShadersManager::destroyAll()
    {
        // Vertex shader
        mLandVS->Release();
        mCommonIL->Release();      
        mLandPS->Release();

        mScreenQuadVS->Release();
        mScreenQuadIL->Release();
        mScreenQuadPS->Release();

        mHorizontalBlurCS->Release();
        mVerticalBlurCS->Release();
    }
}
