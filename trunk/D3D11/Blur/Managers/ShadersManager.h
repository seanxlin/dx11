#pragma once

struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11ComputeShader;
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
        static ID3D11InputLayout* mCommonIL;
        static ID3D11PixelShader* mLandPS;

        static ID3D11VertexShader* mScreenQuadVS;
        static ID3D11InputLayout* mScreenQuadIL;
        static ID3D11PixelShader* mScreenQuadPS;

        static ID3D11ComputeShader* mHorizontalBlurCS;
        static ID3D11ComputeShader* mVerticalBlurCS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
