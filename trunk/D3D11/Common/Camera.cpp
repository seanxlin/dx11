#include "Camera.h"

namespace Utils
{
    void Camera::updateViewMatrix()
    {
	    DirectX::XMVECTOR right = DirectX::XMLoadFloat3(&mRight);
	    DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&mUp);
	    DirectX::XMVECTOR look = DirectX::XMLoadFloat3(&mLook);
	    const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&mPosition);

	    // Keep camera's axes orthogonal to each other and of unit length.
	    look = DirectX::XMVector3Normalize(look);
	    up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(look, right));

	    // up, look already ortho-normal, so no need to normalize cross product.
	    right = DirectX::XMVector3Cross(up, look); 

	    // Fill in the view matrix entries.
	    const float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, right));
	    const float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, up));
	    const float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, look));

	    DirectX::XMStoreFloat3(&mRight, right);
	    DirectX::XMStoreFloat3(&mUp, up);
	    DirectX::XMStoreFloat3(&mLook, look);

	    mView(0,0) = mRight.x; 
	    mView(1,0) = mRight.y; 
	    mView(2,0) = mRight.z; 
	    mView(3,0) = x;   

	    mView(0,1) = mUp.x;
	    mView(1,1) = mUp.y;
	    mView(2,1) = mUp.z;
	    mView(3,1) = y;  

	    mView(0,2) = mLook.x; 
	    mView(1,2) = mLook.y; 
	    mView(2,2) = mLook.z; 
	    mView(3,2) = z;   

	    mView(0,3) = 0.0f;
	    mView(1,3) = 0.0f;
	    mView(2,3) = 0.0f;
	    mView(3,3) = 1.0f;
    }
}


