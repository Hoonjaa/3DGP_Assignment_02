#pragma once
class Camera
{
public:
	Camera();
	~Camera();

	void GenerateLookAtView();
	void GenerateLookToView(XMFLOAT3 CameraPosition, XMFLOAT3 LookDirection);
	void GeneratePerspective();
	void GenerateViewPerspective();



public:
	XMFLOAT4X4 m_4x4View;
	XMFLOAT4X4 m_4x4PerspectiveProject;
	XMFLOAT4X4 m_4x4ViewPerspectiveProject;

private:
	float FOVAngle = XMConvertToRadians(90.0f);
	float nearZ = 0.01f;
	float farZ = 5000.0f;

	float aspect = float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT);
};

