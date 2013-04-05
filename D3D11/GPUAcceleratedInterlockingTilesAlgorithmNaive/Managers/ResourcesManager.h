#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;

struct ShaderResources
{
    ShaderResources()
        : mTerrainDiffuseMapSRV(nullptr)
        , mHeightMapSRV(nullptr)
    {

    }

    ID3D11ShaderResourceView* mTerrainDiffuseMapSRV;
    ID3D11ShaderResourceView* mHeightMapSRV;
};

static ShaderResources gShaderResources;

namespace ShaderResourcesUtils
{
    void initAll(ID3D11Device& device, 
                 ID3D11DeviceContext& context, 
                 ShaderResources& shaderResources);

    void destroyAll(ShaderResources& shaderResources);
}
