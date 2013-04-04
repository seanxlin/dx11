#pragma once

struct ID3D11ComputeShader;
struct ID3D11Device;
struct ID3D11DomainShader;
struct ID3D11HullShader;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;

namespace Managers
{
    class ShadersManager
    {
    public:
        static void initAll(ID3D11Device& device);
        static void destroyAll();

        static ID3D11VertexShader* mTerrainVS;
        static ID3D11InputLayout* mTerrainIL;
        static ID3D11PixelShader* mTerrainPS;
        static ID3D11HullShader* mTerrainHS;
        static ID3D11DomainShader* mTerrainDS;
        static ID3D11ComputeShader* mTerrainCS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
