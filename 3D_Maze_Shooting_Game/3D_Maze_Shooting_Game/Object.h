#pragma once

#include "Mesh.h"
#include "Shader.h"

class Object
{
public:
	Object() { XMStoreFloat4x4(&m_4x4World, XMMatrixIdentity()); }
	virtual ~Object();

public:
	Mesh* m_pMesh = NULL;
	Shader* m_pShader = NULL;
	XMFLOAT4X4 m_4x4World;

	DWORD m_dwColor = RGB(255, 0, 0);

	XMFLOAT3 m_MoveDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float m_MoveSpeed = 0.0f;

	BoundingBox m_BoundingBox;
	bool m_Dead = false;

public:
	virtual void SetMesh(Mesh* pMesh);
	virtual void SetShader(Shader* pShader);
	
	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }
	
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& Position);
	
	void SetMovingDirection(XMFLOAT3& MoveDirection) { XMStoreFloat3(&m_MoveDirection, XMVector3Normalize(XMLoadFloat3(&MoveDirection))); }
	void SetMovingSpeed(float Speed) { m_MoveSpeed = Speed; }

	void Move(XMFLOAT3& Direction, float Speed);

	virtual void UpdateBoundingBox() {}
	void SetDead() { m_Dead = true; }
	bool IsDead() const { return m_Dead; }

	virtual void Update(float DT);
	virtual void Render(ID3D12GraphicsCommandList* CommandList, Camera* pCamera);
};