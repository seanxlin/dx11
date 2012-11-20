#pragma once

struct ID3D11Device;
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

        static ID3D11ShaderResourceView* mOffscreenSRV;
        static ID3D11UnorderedAccessView* mOffscreenUAV;
        static ID3D11RenderTargetView* mOffscreenRTV;

    private:
        ResourcesManager();
        ~ResourcesManager();
        ResourcesManager(const ResourcesManager& resourcesManager);
        const ResourcesManager& operator=(const ResourcesManager& resourcesManager);
    };
}
