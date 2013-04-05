//////////////////////////////////////////////////////////////////////////
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   - It keeps track of the camera coordinate system relative to the world space
//     so that the view matrix can be constructed.  
//   - It keeps track of the viewing frustum of the camera so that the projection
//     matrix can be obtained.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

struct Camera
{
    Camera()
        : mPosition(0.0f, 0.0f, 0.0f)
        , mRight(1.0f, 0.0f, 0.0f)
        , mUp(0.0f, 1.0f, 0.0f)
        , mLook(0.0f, 0.0f, 1.0f)
        , mNearZ(0.0f)
        , mFarZ(0.0f)
        , mAspect(0.0f)
        , mFovY(0.0f)
        , mNearWindowHeight(0.0f)
        , mFarWindowHeight(0.0f)
    {

    }

    // Cache View/Projection matrices.
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProjection;

    // Camera coordinate system with coordinates relative to world space.
    DirectX::XMFLOAT3 mPosition;
    DirectX::XMFLOAT3 mRight;
    DirectX::XMFLOAT3 mUp;
    DirectX::XMFLOAT3 mLook;

    // Cache frustum properties.
    float mNearZ;
    float mFarZ;
    float mAspect;
    float mFovY;
    float mNearWindowHeight;
    float mFarWindowHeight;
};

namespace CameraUtils
{
    void setFrustrum(const float fieldOfViewY, 
        const float aspect, 
        const float nearPlaneZ, 
        const float farPlaneZ,
        Camera& camera);

    void lookAt(const DirectX::XMFLOAT3& position, 
        const DirectX::XMFLOAT3& target, 
        const DirectX::XMFLOAT3& up,
        Camera& camera);

    void strafe(const float distance, Camera& camera);

    void walk(const float distance, Camera& camera);

    void pitch(const float angle, Camera& camera);

    void rotateY(const float angle, Camera& camera);

    void updateViewMatrix(Camera& camera);

    DirectX::XMMATRIX computeViewProjectionMatrix(const Camera& camera);
}  