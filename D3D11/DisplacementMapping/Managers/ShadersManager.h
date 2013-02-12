#pragma once

struct ID3D11Device;
struct ID3D11DomainShader;
struct ID3D11HullShader;
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
        static ID3D11HullShader* mShapesHS;
        static ID3D11DomainShader* mShapesDS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
