#include "PipelineStatesManager.h"

#include <cassert>
#include <D3D11.h>

#include <DxErrorChecker.h>

namespace Managers
{
    ID3D11SamplerState* PipelineStatesManager::mLinearSS =  nullptr;
    ID3D11RasterizerState* PipelineStatesManager::mWireframeRS = nullptr;

    void PipelineStatesManager::initAll(ID3D11Device& device)
    {
        //
        // Anisotropic sampler state
        //
        D3D11_SAMPLER_DESC samplerDesc; 
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; 
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.MipLODBias = 0; 
        samplerDesc.MaxAnisotropy = 16; 
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS; 
        samplerDesc.BorderColor[0] = 0.0f; 
        samplerDesc.BorderColor[1] = 0.0f; 
        samplerDesc.BorderColor[2] = 0.0f; 
        samplerDesc.BorderColor[3] = 0.0f; 
        samplerDesc.MipLODBias = 0.0f; // FLT_MIN 
        samplerDesc.MinLOD = 0.0f; // FLT_MIN 
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // FLT_MAX

        HRESULT result = device.CreateSamplerState(&samplerDesc, &mLinearSS);
        DxErrorChecker(result);

        //
        // Wireframe rasterizer state
        //
        D3D11_RASTERIZER_DESC wireframeDesc;
        ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
        wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireframeDesc.CullMode = D3D11_CULL_BACK;
        wireframeDesc.FrontCounterClockwise = false;
        wireframeDesc.DepthClipEnable = true;

        result = device.CreateRasterizerState(&wireframeDesc, &mWireframeRS);
        DxErrorChecker(result);
    }

    void PipelineStatesManager::destroyAll()
    {
        mLinearSS->Release();
        mWireframeRS->Release();
    }
}