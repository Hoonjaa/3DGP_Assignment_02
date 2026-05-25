#include "stdafx.h"
#include "Object.h"

Object::Object()
{
	XMStoreFloat4x4(&m_4x4World, XMMatrixIdentity());
	// 단위행렬 생성후 저장
}

Object::~Object()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) {
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void Object::SetShader(Shader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void Object::SetMesh(Mesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void Object::SetPosition(float x, float y, float z)
{
	m_4x4World._41 = x;
	m_4x4World._42 = y;
	m_4x4World._43 = z;
}

void Object::SetPosition(XMFLOAT3& Position)
{
	m_4x4World._41 = Position.x;
	m_4x4World._42 = Position.y;
	m_4x4World._43 = Position.z;
}

void Object::Move(XMFLOAT3& Direction, float Speed)
{
	SetPosition(m_4x4World._41 + Direction.x * Speed, m_4x4World._42 + Direction.y * Speed, m_4x4World._43 + Direction.z * Speed);
}

void Object::Update(float DT)
{

}

void Object::Render(ID3D12GraphicsCommandList* CommandList, Camera* pCamera)
{
	if (m_pMesh)
	{
		m_pMesh->Render();
	}
}