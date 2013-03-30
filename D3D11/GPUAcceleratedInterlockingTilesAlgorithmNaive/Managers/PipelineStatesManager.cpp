#include "PipelineStatesManager.h"

#include <cassert>
#include <D3D11.h>

#include <DxErrorChecker.h>

namespace Managers
{
    ID3D11SamplerState* PipelineStatesManager::mAnisotropicSS =  nullptr;
    ID3D11RasterizerState* PipelineStatesManager::mWireframeRS = nullptr;

    void PipelineStatesManager::initAll(ID3D11Device& device)
    {
        //
        // Anisotropic sampler state
        //
        D3D11_SAMPLER_DESC samplerDesc; 
        samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; 
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.MipLODBias = 0; 
        samplerDesc.MaxAnisotropy = 4; 
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
        samplerDesc.BorderColor[0] = 1.0f; 
        samplerDesc.BorderColor[1] = 1.0f; 
        samplerDesc.BorderColor[2] = 1.0f; 
        samplerDesc.BorderColor[3] = 1.0f; 
        samplerDesc.MinLOD = -3.402823466e+38F; // FLT_MIN 
        samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

        HRESULT result = device.CreateSamplerState(&samplerDesc, &mAnisotropicSS);
        DebugUtils::DxErrorChecker(result);

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
        DebugUtils::DxErrorChecker(result);
    }

    void PipelineStatesManager::destroyAll()
    {
        mAnisotropicSS->Release();
        mWireframeRS->Release();
    }
}