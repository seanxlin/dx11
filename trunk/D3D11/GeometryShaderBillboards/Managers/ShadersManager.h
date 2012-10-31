#pragma once

struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11GeometryShader;
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

        static ID3D11VertexShader* mBillboardsVS;
        static ID3D11InputLayout* mBillboardsIL;

        static ID3D11GeometryShader* mBillboardsGS;

        static ID3D11PixelShader* mLandPS;
        static ID3D11PixelShader* mBillboardsPS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
