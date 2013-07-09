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
    Shaders()
        : mVS(nullptr)
        , mIL(nullptr)
        , mPS(nullptr)
        , mCS(nullptr) 
    {

    }

    ID3D11VertexShader* mVS;
    ID3D11InputLayout* mIL;
    ID3D11PixelShader* mPS;
    ID3D11ComputeShader* mCS;
};

namespace ShadersUtils
{
    void initAll(ID3D11Device& device, Shaders& shaders);
    void destroyAll(Shaders& shaders);
}
