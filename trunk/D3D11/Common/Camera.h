//////////////////////////////////////////////////////////////////////////
//
// Simple first person style camera
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

struct Camera
{
    Camera();

    // Cache View/Projection matrices.
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProjection;

    // Camera coordinate system with coordinates 
    // relative to world space.
    DirectX::XMFLOAT3 mPosition;
    DirectX::XMFLOAT3 mRight;
    DirectX::XMFLOAT3 mUp;
    DirectX::XMFLOAT3 mLook;

    // Cache frustum properties.
    float mNearZ;
    float mFarZ;
    float mAspectRatio;
    float mFieldOfViewY;
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

    void setCoordinateSystem(const DirectX::XMFLOAT3& position, 
                             const DirectX::XMFLOAT3& target, 
                             const DirectX::XMFLOAT3& up,
                             Camera& camera);

    void strafe(const float distance, 
                Camera& camera);

    void walk(const float distance, 
              Camera& camera);

    void pitch(const float angle, 
               Camera& camera);

    void rotateAboutYAxis(const float angle, 
                          Camera& camera);

    void updateViewMatrix(Camera& camera);

    DirectX::XMMATRIX computeViewProjectionMatrix(const Camera& camera);
}  