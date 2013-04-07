#pragma once

#include <DirectXMath.h>

#include <Camera.h>
#include <ConstantBuffer.h>
#include <DxErrorChecker.h>
#include <LightHelper.h>

#include "HLSL/Buffers.h"

struct TerrainScene
{
    DirectionalLight mDirectionalLight[3];

    Material mTerrainMaterial;

    ConstantBuffer<GridVSPerFrameBuffer> mGridVSPerObjectBuffer;

    ConstantBuffer<GridHSPerFrameBuffer> mGridHSPerFrameBuffer;

    ConstantBuffer<GridDSPerFrameBuffer> mGridDSPerFrameBuffer;

    ConstantBuffer<GridPSPerFrameBuffer> mGridPSPerFrameBuffer;

    // Define transformations from local spaces to world space.
    DirectX::XMFLOAT4X4 mWorldMatrix;
    DirectX::XMFLOAT4X4 mTextureScaleMatrix;
};

static TerrainScene gTerrainScene;

namespace TerrainSceneUtils
{
    void init(TerrainScene& terrainScene);
    void draw();
}

    class GPUAcceleratedInterlockingTilesAlgorithmNaive
    {
    public:
        GPUAcceleratedInterlockingTilesAlgorithmNaive();

        ~GPUAcceleratedInterlockingTilesAlgorithmNaive();

        bool init();
    };     
        
    int run();

    void updateScene(const float dt);
    
    void drawScene(); 