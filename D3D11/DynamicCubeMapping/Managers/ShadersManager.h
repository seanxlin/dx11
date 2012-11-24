#pragma once

struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;

namespace Managers
{
    class ShadersManager
    {
    public:
        static void initAll(ID3D11Device * const device);
        static void destroyAll();

        static ID3D11VertexShader* mLandVS;
        static ID3D11InputLayout* mLandIL;
        static ID3D11PixelShader* mLandPS;

        static ID3D11VertexShader* mSphereVS;
        static ID3D11InputLayout* mSphereIL;
        static ID3D11PixelShader* mSpherePS;

        static ID3D11VertexShader* mSkyVS;
        static ID3D11InputLayout* mSkyIL;
        static ID3D11PixelShader* mSkyPS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
