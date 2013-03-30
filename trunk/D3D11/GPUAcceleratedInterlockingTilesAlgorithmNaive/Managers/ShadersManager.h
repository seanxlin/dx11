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
        static void initAll(ID3D11Device& device);
        static void destroyAll();

        static ID3D11VertexShader* mTerrainVS;
        static ID3D11InputLayout* mTerrainIL;
        static ID3D11PixelShader* mTerrainPS;

    private:
        ShadersManager();
        ~ShadersManager();
        ShadersManager(const ShadersManager& shadersManager);
        const ShadersManager& operator=(const ShadersManager& shadersManager);
    };
}
