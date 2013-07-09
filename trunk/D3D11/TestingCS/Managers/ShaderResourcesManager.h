#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;

struct ShaderResources
{
    ShaderResources()
        : mDiffuseMapSRV(nullptr)
        , mCSResultsSRV(nullptr)
        , mCSResultsUAV(nullptr)
    {

    }

    ID3D11ShaderResourceView* mDiffuseMapSRV;
    ID3D11ShaderResourceView* mCSResultsSRV;
    ID3D11UnorderedAccessView* mCSResultsUAV;
};

namespace ShaderResourcesUtils
{
    void initAll(ID3D11Device& device, 
                 ID3D11DeviceContext& context, 
                 ShaderResources& shaderResources);

    void destroyAll(ShaderResources& shaderResources);
}
