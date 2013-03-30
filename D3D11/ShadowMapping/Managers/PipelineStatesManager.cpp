#include "PipelineStatesManager.h"

#include <cassert>
#include <D3D11.h>

#include <DxErrorChecker.h>

namespace Managers
{
    ID3D11SamplerState* PipelineStatesManager::mAnisotropicSS =  nullptr;

    ID3D11RasterizerState* PipelineStatesManager::mWireframeRS = nullptr;
    ID3D11RasterizerState* PipelineStatesManager::mNoCullRS = nullptr;
    ID3D11RasterizerState* PipelineStatesManager::mDepthRS = nullptr;

    ID3D11DepthStencilState* PipelineStatesManager::mLessEqualDSS = nullptr;

    ID3D11BlendState* PipelineStatesManager::mAlphaToCoverageBS = nullptr;
    ID3D11BlendState* PipelineStatesManager::mTransparentBS = nullptr;

    void PipelineStatesManager::initAll(ID3D11Device* device)
    {
        assert(device);
        
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

        HRESULT result = device->CreateSamplerState(&samplerDesc, &mAnisotropicSS);
        DebugUtils::DxErrorChecker(result);

        //
        // Wireframe rasterizer state
        //
        D3D11_RASTERIZER_DESC rasterizerDesc;
        ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = false;
        rasterizerDesc.DepthClipEnable = true;

        result = device->CreateRasterizerState(&rasterizerDesc, &mWireframeRS);
        DebugUtils::DxErrorChecker(result);

        //
        // NoCullRS
        //
        ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        rasterizerDesc.FrontCounterClockwise = false;
        rasterizerDesc.DepthClipEnable = true;

        result = device->CreateRasterizerState(&rasterizerDesc, &mNoCullRS);
        DebugUtils::DxErrorChecker(result);

        //
        // Depth Rasterizer state
        // 
        // If the depth buffer currently bound to the output-merger stage has a UNORM format or
        // no depth buffer is bound the bias value is calculated like this: 
        //
        // Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
        //
        // where r is the minimum representable value > 0 in the depth-buffer format converted to float32.
        // 
        // For a 24-bit depth buffer, r = 1 / 2^24.
        //
        // Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006
        ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = false;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.DepthBias = 100000;
        rasterizerDesc.DepthBiasClamp = 0.0f;
        rasterizerDesc.SlopeScaledDepthBias = 1.0f;

        result = device->CreateRasterizerState(&rasterizerDesc, &mDepthRS);
        DebugUtils::DxErrorChecker(result);

        //
        // LessEqual depth stencil state
        //
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp =  D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

        result = device->CreateDepthStencilState(&depthStencilDesc, &mLessEqualDSS);
        DebugUtils::DxErrorChecker(result);

        //
        // AlphaToCoverageBS
        //
        D3D11_BLEND_DESC alphaToCoverageDesc = {0};
        alphaToCoverageDesc.AlphaToCoverageEnable = true;
        alphaToCoverageDesc.IndependentBlendEnable = false;
        alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
        alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        result = device->CreateBlendState(&alphaToCoverageDesc, &mAlphaToCoverageBS);
        DebugUtils::DxErrorChecker(result);

        //
        // TransparentBS
        //

        D3D11_BLEND_DESC transparentDesc = {0};
        transparentDesc.AlphaToCoverageEnable = false;
        transparentDesc.IndependentBlendEnable = false;
        transparentDesc.RenderTarget[0].BlendEnable = true;
        transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        result = device->CreateBlendState(&transparentDesc, &mTransparentBS);
        DebugUtils::DxErrorChecker(result);
    }

    void PipelineStatesManager::destroyAll()
    {
        mAnisotropicSS->Release();

        mWireframeRS->Release();
        mNoCullRS->Release();
        mDepthRS->Release();

        mLessEqualDSS->Release();

        mAlphaToCoverageBS->Release();
        mTransparentBS->Release();
    }
}