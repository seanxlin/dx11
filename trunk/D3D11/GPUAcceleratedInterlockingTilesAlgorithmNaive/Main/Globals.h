#include <Managers/GeometryBuffersManager.h>
#include <Managers/PipelineStatesManager.h>
#include <Managers/ShaderResourcesManager.h>
#include <Managers/ShadersManager.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

struct Globals
{
    Shaders mShaders;
    ShaderResources mShaderResources;
    PipelineStates mPipelineStates;
    GeometryBuffers mGeometryBuffers;
};

static Globals gGlobals;

namespace GlobalsUtils
{
    void init(ID3D11Device& device, 
              ID3D11DeviceContext& context, 
              Globals& globals);
    void destroy(Globals& globals);
}