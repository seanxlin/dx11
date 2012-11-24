#pragma once

struct ID3D11Device;
struct ID3D11DepthStencilView;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;

namespace Managers
{
    class ResourcesManager
    {
    public:
        static void initAll(ID3D11Device* device, ID3D11DeviceContext* context);
        static void destroyAll();

        static ID3D11ShaderResourceView* mSandSRV;
        static ID3D11ShaderResourceView* mSkyCubeMapSRV;
        static ID3D11ShaderResourceView* mSphereDiffuseMapSRV;

        static ID3D11DepthStencilView* mDynamicCubeMapDSV;
        static ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
        static ID3D11ShaderResourceView* mDynamicCubeMapSRV;

    private:
        ResourcesManager();
        ~ResourcesManager();
        ResourcesManager(const ResourcesManager& resourcesManager);
        const ResourcesManager& operator=(const ResourcesManager& resourcesManager);

        static void buildDynamicCubeMapViews(ID3D11Device* device);
    };
}
