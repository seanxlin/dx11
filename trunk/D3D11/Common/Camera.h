//////////////////////////////////////////////////////////////////////////
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

namespace Utils
{
    class Camera
    {
    public:
        inline Camera();
        inline ~Camera();

        // Get/Set world camera position.
        inline DirectX::XMVECTOR positionXM() const;
        inline DirectX::XMFLOAT3 position() const;
        inline void setPosition(const float x, const float y, const float z);
        inline void setPosition(const DirectX::XMFLOAT3& newPosition);

        // Get camera basis vectors.
        inline DirectX::XMVECTOR rightXM() const;
        inline DirectX::XMFLOAT3 right() const;
        inline DirectX::XMVECTOR upXM() const;
        inline DirectX::XMFLOAT3 up() const;
        inline DirectX::XMVECTOR lookXM() const;
        inline DirectX::XMFLOAT3 look() const;

        // Get frustum properties.
        inline float nearZ() const;
        inline float farZ() const;
        inline float aspect() const;
        inline float fovY() const;
        inline float fovX() const;

        // Get near and far plane dimensions in view space coordinates.
        inline float nearWindowWidth() const;
        inline float nearWindowHeight() const;
        inline float farWindowWidth() const;
        inline float farWindowHeight() const;

        // Set frustum.
        inline void setLens(const float fovY, const float aspect, const float nearPlaneZ, const float farPlaneZ);

        // Define camera space via LookAt parameters.
        inline void lookAt(const DirectX::FXMVECTOR position, const DirectX::FXMVECTOR target, const DirectX::FXMVECTOR worldUp);
        inline void lookAt(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

        // Get View/Proj matrices.
        inline DirectX::XMMATRIX view() const;
        inline DirectX::XMMATRIX projection() const;
        inline DirectX::XMMATRIX viewProjection() const;

        // Strafe/Walk the camera a distance d.
        inline void strafe(const float distance);
        inline void walk(const float distance);

        // Rotate the camera.
        inline void pitch(const float angle);
        inline void rotateY(const float angle);

        // After modifying camera position/orientation, call to rebuild the view matrix.
        void updateViewMatrix();

    private:
        // Cache View/Proj matrices.
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

    inline Camera::Camera()
        : mPosition(0.0f, 0.0f, 0.0f)
        , mRight(1.0f, 0.0f, 0.0f)
        , mUp(0.0f, 1.0f, 0.0f)
        , mLook(0.0f, 0.0f, 1.0f)
    {
        setLens(0.25f * DirectX::XM_PI, 1.0f, 1.0f, 1000.0f);
    }

    inline Camera::~Camera()
    {
    
    }

    inline DirectX::XMVECTOR Camera::positionXM() const
    {
        return DirectX::XMLoadFloat3(&mPosition);
    }

    inline DirectX::XMFLOAT3 Camera::position() const
    {
        return mPosition;
    }

    inline void Camera::setPosition(const float x, const float y, const float z)
    {
        mPosition = DirectX::XMFLOAT3(x, y, z);
    }

    inline void Camera::setPosition(const DirectX::XMFLOAT3& newPosition)
    {
        mPosition = newPosition;
    }

    inline DirectX::XMVECTOR Camera::rightXM() const
    {
        return DirectX::XMLoadFloat3(&mRight);
    }

    inline DirectX::XMFLOAT3 Camera::right() const
    {
        return mRight;
    }

    inline DirectX::XMVECTOR Camera::upXM() const
    {
        return DirectX::XMLoadFloat3(&mUp);
    }

    inline DirectX::XMFLOAT3 Camera::up() const
    {
        return mUp;
    }

    inline DirectX::XMVECTOR Camera::lookXM() const
    {
        return DirectX::XMLoadFloat3(&mLook);
    }

    inline DirectX::XMFLOAT3 Camera::look() const
    {
        return mLook;
    }

    inline float Camera::nearZ() const
    {
        return mNearZ;
    }

    inline float Camera::farZ() const
    {
        return mFarZ;
    }

    inline float Camera::aspect() const
    {
        return mAspect;
    }

    inline float Camera::fovY() const
    {
        return mFovY;
    }

    inline float Camera::fovX() const
    {
        float halfWidth = 0.5f * nearWindowWidth();

        return 2.0f * atan(halfWidth / mNearZ);
    }

    inline float Camera::nearWindowWidth() const
    {
        return mAspect * mNearWindowHeight;
    }

    inline float Camera::nearWindowHeight() const
    {
        return mNearWindowHeight;
    }

    inline float Camera::farWindowWidth() const
    {
        return mAspect * mFarWindowHeight;
    }

    inline float Camera::farWindowHeight() const
    {
        return mFarWindowHeight;
    }

    inline void Camera::setLens(const float fovY, const float aspect, const float nearPlaneZ, const float farPlaneZ)
    {
        // cache properties
        mFovY = fovY;
        mAspect = aspect;
        mNearZ = nearPlaneZ;
        mFarZ = farPlaneZ;

        mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
        mFarWindowHeight  = 2.0f * mFarZ * tanf(0.5f * mFovY);

        DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
        DirectX::XMStoreFloat4x4(&mProjection, projectionMatrix);
    }

    inline void Camera::lookAt(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp)
    {
        DirectX::XMVECTOR look = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
        DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, look));
        DirectX::XMVECTOR up = DirectX::XMVector3Cross(look, right);

        DirectX::XMStoreFloat3(&mPosition, position);
        DirectX::XMStoreFloat3(&mLook, look);
        DirectX::XMStoreFloat3(&mRight, right);
        DirectX::XMStoreFloat3(&mUp, up);
    }

    inline void Camera::lookAt(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
    {
        DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&position);
        DirectX::XMVECTOR targetVec = DirectX::XMLoadFloat3(&target);
        DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

        lookAt(positionVec, targetVec, upVec);
    }

    inline DirectX::XMMATRIX Camera::view() const
    {
        return DirectX::XMLoadFloat4x4(&mView);
    }

    inline DirectX::XMMATRIX Camera::projection() const
    {
        return DirectX::XMLoadFloat4x4(&mProjection);
    }

    inline DirectX::XMMATRIX Camera::viewProjection() const
    {
        return DirectX::XMMatrixMultiply(view(), projection());
    }

    inline void Camera::strafe(const float distance)
    {
        // mPosition += distance * mRight
        DirectX::XMVECTOR s = DirectX::XMVectorReplicate(distance);
        DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&mRight);
        DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&mPosition);
        DirectX::XMStoreFloat3(&mPosition, DirectX::XMVectorMultiplyAdd(s, r, p));
    }

    inline void Camera::walk(const float distance)
    {
        // mPosition += distance * mLook
        DirectX::XMVECTOR s = DirectX::XMVectorReplicate(distance);
        DirectX::XMVECTOR l = DirectX::XMLoadFloat3(&mLook);
        DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&mPosition);
        DirectX::XMStoreFloat3(&mPosition, DirectX::XMVectorMultiplyAdd(s, l, p));
    }

    inline void Camera::pitch(const float angle)
    {
        // Rotate up and look vector about the right vector.
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&mRight), angle);

        DirectX::XMStoreFloat3(&mUp, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&mUp), rotationMatrix));
        DirectX::XMStoreFloat3(&mLook, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&mLook), rotationMatrix));
    }

    inline void Camera::rotateY(const float angle)
    {
        // Rotate the basis vectors about the world y-axis.
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(angle);

        DirectX::XMStoreFloat3(&mRight, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&mRight), rotationMatrix));
        DirectX::XMStoreFloat3(&mUp, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&mUp), rotationMatrix));
        DirectX::XMStoreFloat3(&mLook, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&mLook), rotationMatrix));
    }
}