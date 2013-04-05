#pragma once

struct ID3D11Device;
struct ID3D11RasterizerState;
struct ID3D11SamplerState;

struct PipelineStates
{
    PipelineStates()
        : mLinearSS(nullptr)
        , mWireframeRS(nullptr)
    {

    }

    ID3D11SamplerState* mLinearSS;
    ID3D11RasterizerState* mWireframeRS;
};

static PipelineStates gPipelineStates;

namespace PipelineStatesUtils
{
    void initAll(ID3D11Device& device, PipelineStates& pipelineStates);
    void destroyAll(PipelineStates& pipelineStates);
}