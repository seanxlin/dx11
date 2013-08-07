//////////////////////////////////////////////////////////////////////////
//
// Structures used for lighting
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// Make sure structure alignment agrees with HLSL structure padding rules. 
// Elements are packed into 4D vectors with the restriction that an element
// cannot straddle a 4D vector boundary.
//////////////////////////////////////////////////////////////////////////

struct DirectionalLight
{
    DirectionalLight() 
        : mAmbient(0.0f, 0.0f, 0.0f, 0.0f)
        , mDiffuse(0.0f, 0.0f, 0.0f, 0.0f)
        , mSpecular(0.0f, 0.0f, 0.0f, 0.0f)
        , mDirection(0.0f, 0.0f, 0.0f)
        , mPad(0.0f)
    { 

    }

    DirectX::XMFLOAT4 mAmbient;
    DirectX::XMFLOAT4 mDiffuse;
    DirectX::XMFLOAT4 mSpecular;
    DirectX::XMFLOAT3 mDirection;
    float mPad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
    PointLight() 
        : mAmbient(0.0f, 0.0f, 0.0f, 0.0f)
        , mDiffuse(0.0f, 0.0f, 0.0f, 0.0f)
        , mSpecular(0.0f, 0.0f, 0.0f, 0.0f)
        , mPosition(0.0f, 0.0f, 0.0f)
        , mRange(0.0f)
        , mAttenuation(0.0f, 0.0f, 0.0f)
        , mPad(0.0f)
    { 

    }

    DirectX::XMFLOAT4 mAmbient;
    DirectX::XMFLOAT4 mDiffuse;
    DirectX::XMFLOAT4 mSpecular;

    // Packed into 4D vector: (Position, Range)
    DirectX::XMFLOAT3 mPosition;
    float mRange;

    // Packed into 4D vector: (A0, A1, A2, Pad)
    DirectX::XMFLOAT3 mAttenuation;
    float mPad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
    SpotLight() 
        : mAmbient(0.0f, 0.0f, 0.0f, 0.0f)
        , mDiffuse(0.0f, 0.0f, 0.0f, 0.0f)
        , mSpecular(0.0f, 0.0f, 0.0f, 0.0f)
        , mPosition(0.0f, 0.0f, 0.0f)
        , mRange(0.0f)
        , mDirection(0.0f, 0.0f, 0.0f)
        , mSpot(0.0f)
        , mAttenuation(0.0f, 0.0f, 0.0f)
        , mPad(0.0f)
    { 

    }

    DirectX::XMFLOAT4 mAmbient;
    DirectX::XMFLOAT4 mDiffuse;
    DirectX::XMFLOAT4 mSpecular;

    // Packed into 4D vector: (Position, Range)
    DirectX::XMFLOAT3 mPosition;
    float mRange;

    // Packed into 4D vector: (Direction, Spot)
    DirectX::XMFLOAT3 mDirection;
    float mSpot;

    // Packed into 4D vector: (Att, Pad)
    DirectX::XMFLOAT3 mAttenuation;
    float mPad; // Pad the last float so we can set an array of lights if we wanted.
};

struct Material
{
    Material() 
        : mAmbient(0.0f, 0.0f, 0.0f, 0.0f)
        , mDiffuse(0.0f, 0.0f, 0.0f, 0.0f)
        , mSpecular(0.0f, 0.0f, 0.0f, 0.0f)
        , mReflect(0.0f, 0.0f, 0.0f, 0.0f)
    { 

    }

    DirectX::XMFLOAT4 mAmbient;
    DirectX::XMFLOAT4 mDiffuse;
    DirectX::XMFLOAT4 mSpecular; // w = SpecPower
    DirectX::XMFLOAT4 mReflect;
};