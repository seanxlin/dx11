//////////////////////////////////////////////////////////////////////////
//
// Manages initialization, destruction and access to pipeline states
//
//////////////////////////////////////////////////////////////////////////

#pragma once

struct ID3D11Device;
struct ID3D11RasterizerState;
struct ID3D11SamplerState;

struct PipelineStates
{
    PipelineStates();

    ID3D11SamplerState* mLinearClampSS;
    ID3D11SamplerState* mLinearWrapSS;
    ID3D11RasterizerState* mWireframeRS;
};

namespace PipelineStatesUtils
{
    void init(ID3D11Device& device, PipelineStates& pipelineStates);
    void destroy(PipelineStates& pipelineStates);
}