#include "Globals.h"

namespace GlobalsUtils
{
    void init(ID3D11Device& device, 
              ID3D11DeviceContext& context, 
              Globals& globals)
    {
        ShadersUtils::initAll(device, globals.mShaders);

        ShaderResourcesUtils::initAll(
            device, 
            context, 
            globals.mShaderResources);
        
        PipelineStatesUtils::initAll(device, globals.mPipelineStates);

        GeometryBuffersUtils::initAll(device, globals.mGeometryBuffers);
    }

    void destroy(Globals& globals)
    {
        ShadersUtils::destroyAll(globals.mShaders);

        ShaderResourcesUtils::destroyAll(globals.mShaderResources);

        PipelineStatesUtils::destroyAll(globals.mPipelineStates);

        GeometryBuffersUtils::destroyAll(globals.mGeometryBuffers);
    }
}