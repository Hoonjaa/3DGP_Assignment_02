#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
{
	XMStoreFloat4x4(&m_4x4View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_4x4PerspectiveProject, XMMatrixIdentity());
	XMStoreFloat4x4(&m_4x4ViewPerspectiveProject, XMMatrixIdentity());
}

Camera::~Camera()
{

}

void Camera::GenerateLookAtView()
{
    XMVECTOR eyePosition = XMVectorSet(5.0f, 0.0f, -10.0f, 1.0f);
    XMVECTOR focusPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // 왼손 좌표계용 뷰 행렬 생성
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);

    // 생성된 뷰 행렬을 클래스 멤버인 m_4x4View에 저장
    XMStoreFloat4x4(&m_4x4View, viewMatrix);
}

void Camera::GenerateLookToView(XMFLOAT3 CameraPosition, XMFLOAT3 LookDirection)
{
    XMVECTOR eyePosition = XMVectorSet(CameraPosition.x, CameraPosition.y, CameraPosition.z, 1.0f);
    XMVECTOR focusDirection = XMVectorSet(LookDirection.x, LookDirection.y, LookDirection.z, 0.0f);
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // 왼손 좌표계용 뷰 행렬 생성
    XMMATRIX viewMatrix = XMMatrixLookToLH(eyePosition, focusDirection, upDirection);

    // 생성된 뷰 행렬을 클래스 멤버인 m_4x4View에 저장
    XMStoreFloat4x4(&m_4x4View, viewMatrix);
}

void Camera::GeneratePerspective()
{
    XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(FOVAngle, aspect, nearZ, farZ);
    XMStoreFloat4x4(&m_4x4PerspectiveProject, projMatrix);
}

void Camera::GenerateViewPerspective()
{
    XMMATRIX view = XMLoadFloat4x4(&m_4x4View);
    XMMATRIX proj = XMLoadFloat4x4(&m_4x4PerspectiveProject);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMStoreFloat4x4(&m_4x4ViewPerspectiveProject, viewProj);
}
