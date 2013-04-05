#include "Camera.h"

namespace
{
    void lookAt(DirectX::FXMVECTOR position, 
                DirectX::FXMVECTOR target, 
                DirectX::FXMVECTOR worldUp,
                Camera& camera)
    {
        const DirectX::XMVECTOR look = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
        const DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, look));
        const DirectX::XMVECTOR up = DirectX::XMVector3Cross(look, right);

        DirectX::XMStoreFloat3(&camera.mPosition, position);
        DirectX::XMStoreFloat3(&camera.mLook, look);
        DirectX::XMStoreFloat3(&camera.mRight, right);
        DirectX::XMStoreFloat3(&camera.mUp, up);
    }
}

namespace CameraUtils
{
    void setFrustrum(const float fieldOfViewY, 
                     const float aspect, 
                     const float nearPlaneZ, 
                     const float farPlaneZ,
                     Camera& camera)
    {
        // Cache lens properties
        camera.mFovY = fieldOfViewY;
        camera.mAspect = aspect;
        camera.mNearZ = nearPlaneZ;
        camera.mFarZ = farPlaneZ;

        const float halfFovY = 0.5f * camera.mFovY;
        camera.mNearWindowHeight = 2.0f * camera.mNearZ * tanf(halfFovY);
        camera.mFarWindowHeight  = 2.0f * camera.mFarZ * tanf(halfFovY);

        DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(camera.mFovY, 
            camera.mAspect, 
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
        up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(look, right));

        // up, look already ortho-normal, so no need to normalize cross product.
        right = DirectX::XMVector3Cross(up, look); 

        // Fill in the view matrix entries.
        const float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, right));
        const float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, up));
        const float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, look));

        DirectX::XMStoreFloat3(&camera.mRight, right);
        DirectX::XMStoreFloat3(&camera.mUp, up);
        DirectX::XMStoreFloat3(&camera.mLook, look);

        camera.mView(0,0) = camera.mRight.x; 
        camera.mView(1,0) = camera.mRight.y; 
        camera.mView(2,0) = camera.mRight.z; 
        camera.mView(3,0) = x;   

        camera.mView(0,1) = camera.mUp.x;
        camera.mView(1,1) = camera.mUp.y;
        camera.mView(2,1) = camera.mUp.z;
        camera.mView(3,1) = y;  

        camera.mView(0,2) = camera.mLook.x; 
        camera.mView(1,2) = camera.mLook.y; 
        camera.mView(2,2) = camera.mLook.z; 
        camera.mView(3,2) = z;   

        camera.mView(0,3) = 0.0f;
        camera.mView(1,3) = 0.0f;
        camera.mView(2,3) = 0.0f;
        camera.mView(3,3) = 1.0f;
    }

    DirectX::XMMATRIX computeViewProjectionMatrix(const Camera& camera)
    {
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&camera.mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&camera.mProjection);

        return DirectX::XMMatrixMultiply(view, projection);
    }

    void lookAt(const DirectX::XMFLOAT3& position, 
                const DirectX::XMFLOAT3& target, 
                const DirectX::XMFLOAT3& up,
                Camera& camera)
    {
        const DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&position);
        const DirectX::XMVECTOR targetVec = DirectX::XMLoadFloat3(&target);
        DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

        const DirectX::XMVECTOR look = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(targetVec, positionVec));
        const DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(upVec, look));
        upVec = DirectX::XMVector3Cross(look, right);

        DirectX::XMStoreFloat3(&camera.mPosition, positionVec);
        DirectX::XMStoreFloat3(&camera.mLook, look);
        DirectX::XMStoreFloat3(&camera.mRight, right);
        DirectX::XMStoreFloat3(&camera.mUp, upVec);
    }

    void strafe(const float distance, 
                Camera& camera)
    {
        // mPosition += distance * mRight
        const DirectX::XMVECTOR s = DirectX::XMVectorReplicate(distance);
        const DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&camera.mRight);
        const DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&camera.mPosition);
        DirectX::XMStoreFloat3(&camera.mPosition, DirectX::XMVectorMultiplyAdd(s, r, p));
    }

    void walk(const float distance, 
              Camera& camera)
    {
        // mPosition += distance * mLook
        const DirectX::XMVECTOR distanceVector = DirectX::XMVectorReplicate(distance);
        const DirectX::XMVECTOR lookVector = DirectX::XMLoadFloat3(&camera.mLook);
        const DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&camera.mPosition);
        DirectX::XMStoreFloat3(&camera.mPosition, DirectX::XMVectorMultiplyAdd(distanceVector, lookVector, positionVector));
    }

    void pitch(const float angle,
               Camera& camera)
    {
        // Rotate up and look vector about the right vector.
        const  DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&camera.mRight), angle);

        DirectX::XMStoreFloat3(&camera.mUp, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mUp), rotationMatrix));
        DirectX::XMStoreFloat3(&camera.mLook, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mLook), rotationMatrix));
    }

    void rotateY(const float angle,
                 Camera& camera)
    {
        // Rotate the basis vectors about the world y-axis.
        const DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(angle);

        DirectX::XMStoreFloat3(&camera.mRight, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mRight), rotationMatrix));
        DirectX::XMStoreFloat3(&camera.mUp, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mUp), rotationMatrix));
        DirectX::XMStoreFloat3(&camera.mLook, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&camera.mLook), rotationMatrix));
    }
}