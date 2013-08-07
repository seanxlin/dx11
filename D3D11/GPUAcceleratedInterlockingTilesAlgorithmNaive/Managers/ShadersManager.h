//////////////////////////////////////////////////////////////////////////
//
// Manager class to initialize, destroy and access to shaders and
// input layouts
//
//////////////////////////////////////////////////////////////////////////

#pragma once

struct ID3D11ComputeShader;
struct ID3D11Device;
struct ID3D11DomainShader;
struct ID3D11HullShader;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;

struct Shaders
{
    Shaders();

    ID3D11VertexShader* mTerrainVS;
    ID3D11InputLayout* mTerrainIL;
    ID3D11PixelShader* mTerrainPS;
    ID3D11HullShader* mTerrainHS;
    ID3D11DomainShader* mTerrainDS;
};

namespace ShadersUtils
{
    void initAll(ID3D11Device& device, Shaders& shaders);
    void destroyAll(Shaders& shaders);
}
