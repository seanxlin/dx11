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

        static ID3D11VertexShader* mBezierSurfaceVS;
        static ID3D11InputLayout* mBezierSurfaceIL;
        static ID3D11PixelShader* mBezierSurfacePS;
        static ID3D11HullShader* mBezierSurfaceHS;
        static ID3D11DomainShader* mBezierSurfaceDS;


    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
