#include "Camera.h"

Camera::Camera()
    : mPosition(0.0f, 0.0f, 0.0f)
    , mRight(1.0f, 0.0f, 0.0f)
    , mUp(0.0f, 1.0f, 0.0f)
    , mLook(0.0f, 0.0f, 1.0f)
    , mNearZ(0.0f)
    , mFarZ(0.0f)
    , mAspectRatio(0.0f)
    , mFieldOfViewY(0.0f)
    , mNearWindowHeight(0.0f)
    , mFarWindowHeight(0.0f)
{

}

namespace CameraUtils
{
    void setFrustrum(const float fieldOfViewY, 
                     const float aspect, 
                     const float nearPlaneZ,
                     const float farPlaneZ,
                     Camera& camera)
    {
        // Cache frustrum properties
        camera.mFieldOfViewY = fieldOfViewY;
        camera.mAspectRatio = aspect;
        camera.mNearZ = nearPlaneZ;
        camera.mFarZ = farPlaneZ;
        const float halfFieldOfViewY = 0.5f * camera.mFieldOfViewY;
        camera.mNearWindowHeight = 2.0f * camera.mNearZ * tanf(halfFieldOfViewY);
        camera.mFarWindowHeight  = 2.0f * camera.mFarZ * tanf(halfFieldOfViewY);

        // Cache projection matrix
        DirectX::XMMATRIX projectionMatrix = 
            DirectX::XMMatrixPerspectiveFovLH(camera.mFieldOfViewY, 
                                              camera.mAspectRatio, 
                                              camera.mNearZ,
                                              camera.mFarZ);
        DirectX::XMStoreFloat4x4(&camera.mProjection, projectionMatrix);
    }

    void updateViewMatrix(Camera& camera)
    {
        DirectX::XMVECTOR right = DirectX::XMLoadFloat3(&camera.mRight);
        DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera.mUp);
        DirectX::XMVECTOR look = DirectX::XMLoadFloat3(&camera.mLook);
        const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&camera.mPosition);

        // Keep camera's axes orthogonal to each other and of unit length.
        look = DirectX::XMVector3Normalize(look);
        DirectX::XMStoreFloat3(&camera.mLook, look);

        up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(look, right));
        DirectX::XMStoreFloat3(&camera.mUp, up);
        
        right = DirectX::XMVector3Cross(up, look); 
        DirectX::XMStoreFloat3(&camera.mRight, right);
        
        // Fill in the view matrix entries.
        const float x = -1.0f * DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, right));
        const float y = -1.0f * DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, up));
        const float z = -1.0f * DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, look));

        camera.mView(0, 0) = camera.mRight.x; 
        camera.mView(1, 0) = camera.mRight.y; 
        camera.mView(2, 0) = camera.mRight.z; 
        camera.mView(3, 0) = x;   

        camera.mView(0, 1) = camera.mUp.x;
        camera.mView(1, 1) = camera.mUp.y;
        camera.mView(2, 1) = camera.mUp.z;
        camera.mView(3, 1) = y;  

        camera.mView(0, 2) = camera.mLook.x; 
        camera.mView(1, 2) = camera.mLook.y; 
        camera.mView(2, 2) = camera.mLook.z; 
        camera.mView(3, 2) = z;   

        camera.mView(0, 3) = 0.0f;
        camera.mView(1, 3) = 0.0f;
        camera.mView(2, 3) = 0.0f;
        camera.mView(3, 3) = 1.0f;
    }

    DirectX::XMMATRIX computeViewProjectionMatrix(const Camera& camera)
    {
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&camera.mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&camera.mProjection);

        return DirectX::XMMatrixMultiply(view, projection);
    }

    void setCoordinateSystem(const DirectX::XMFLOAT3& position, 
                             const DirectX::XMFLOAT3& target, 
                             const DirectX::XMFLOAT3& up,
                             Camera& camera)
    {
        const DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        DirectX::XMStoreFloat3(&camera.mPosition, positionVector);

        const DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat3(&target);
        DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&up);

        const DirectX::XMVECTOR look = 
            DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(targetVector, positionVector));
        DirectX::XMStoreFloat3(&camera.mLook, look);
        
        const DirectX::XMVECTOR right = 
            DirectX::XMVector3Normalize(DirectX::XMVector3Cross(upVector, look));
        DirectX::XMStoreFloat3(&camera.mRight, right);

        upVector = DirectX::XMVector3Cross(look, right);
        DirectX::XMStoreFloat3(&camera.mUp, upVector);
    }

    void strafe(const float distance, 
                Camera& camera)
    {
        const DirectX::XMVECTOR distanceVector = DirectX::XMVectorReplicate(distance);
        const DirectX::XMVECTOR right = DirectX::XMLoadFloat3(&camera.mRight);
        const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&camera.mPosition);
        const DirectX::XMVECTOR newPosition = DirectX::XMVectorMultiplyAdd(distanceVector,
                                                                           right,
                                                                           position);
        DirectX::XMStoreFloat3(&camera.mPosition, newPosition);
    }

    void walk(const float distance, 
              Camera& camera)
    {
        const DirectX::XMVECTOR distanceVector = DirectX::XMVectorReplicate(distance);
        const DirectX::XMVECTOR look = DirectX::XMLoadFloat3(&camera.mLook);
        const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&camera.mPosition);
        const DirectX::XMVECTOR newPosition = DirectX::XMVectorMultiplyAdd(distanceVector,
                                                                           look,
                                                                           position);
        DirectX::XMStoreFloat3(&camera.mPosition, newPosition);
    }

    void pitch(const float angle,
               Camera& camera)
    {
        // We should rotate up and look vectors about right vector.

        const DirectX::XMMATRIX rotationMatrix = 
            DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&camera.mRight), angle);

        const DirectX::XMVECTOR newUp = 
            DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mUp), 
                                              rotationMatrix);
        DirectX::XMStoreFloat3(&camera.mUp, newUp);

        const DirectX::XMVECTOR newLook = 
            DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mLook), 
                                              rotationMatrix);
        DirectX::XMStoreFloat3(&camera.mLook, newLook);
    }

    void rotateAboutYAxis(const float angle,
                          Camera& camera)
    {
        // We should rotate the basis vectors (right, up and look) 
        // about the world y-axis.

        const DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(angle);

        const DirectX::XMVECTOR newRight = 
            DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mRight), 
                                              rotationMatrix);
        DirectX::XMStoreFloat3(&camera.mRight, newRight);

        const DirectX::XMVECTOR newUp = 
            DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mUp), 
                                              rotationMatrix);
        DirectX::XMStoreFloat3(&camera.mUp, newUp);

        const DirectX::XMVECTOR newLook =
            DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mLook), 
                                              rotationMatrix);
        DirectX::XMStoreFloat3(&camera.mLook, newLook);
    }
}