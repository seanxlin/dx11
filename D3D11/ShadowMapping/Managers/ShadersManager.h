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

        static ID3D11VertexShader* mShapesVS;
        static ID3D11InputLayout* mShapesIL;
        static ID3D11PixelShader* mShapesPS;

        static ID3D11VertexShader* mFloorVS;
        static ID3D11InputLayout* mFloorIL;
        static ID3D11PixelShader* mFloorPS;

        static ID3D11VertexShader* mShadowMapVS;
        static ID3D11InputLayout* mShadowMapIL;
        static ID3D11PixelShader* mShadowMapPS;

        static ID3D11VertexShader* mFloorShadowMapVS;
        static ID3D11InputLayout* mFloorShadowMapIL;
        static ID3D11PixelShader* mFloorShadowMapPS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
