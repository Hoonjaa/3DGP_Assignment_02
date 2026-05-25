#include "stdafx.h"
#include "Mesh.h"

void Mesh::Render(ID3D12GraphicsCommandList* CommandList, XMFLOAT4X4* pMtxWorld, Camera* pCamera, DWORD& color)
{
	XMMATRIX mtxWorld = XMLoadFloat4x4(pMtxWorld);
	XMMATRIX mtxViewProj = XMLoadFloat4x4(&pCamera->m_4x4ViewPerspectiveProject);

	DWORD faceColor = color;
}

CubeMesh::CubeMesh(float fWidth, float fHeight, float fDepth)
{
	float w = fWidth * 0.5f;
	float h = fHeight * 0.5f;
	float d = fDepth * 0.5f;

	m_Vertices.push_back(CVertex(-w, +h, -d)); // 0: 앞 위 왼쪽
	m_Vertices.push_back(CVertex(+w, +h, -d)); // 1: 앞 위 오른쪽
	m_Vertices.push_back(CVertex(+w, -h, -d)); // 2: 앞 아래 오른쪽
	m_Vertices.push_back(CVertex(-w, -h, -d)); // 3: 앞 아래 왼쪽
	m_Vertices.push_back(CVertex(-w, +h, +d)); // 4: 뒤 위 왼쪽
	m_Vertices.push_back(CVertex(+w, +h, +d)); // 5: 뒤 위 오른쪽
	m_Vertices.push_back(CVertex(+w, -h, +d)); // 6: 뒤 아래 오른쪽
	m_Vertices.push_back(CVertex(-w, -h, +d)); // 7: 뒤 아래 왼쪽

	m_Indices = {
		// 앞면
		0, 1, 2,   0, 2, 3,
		// 오른쪽면
		1, 5, 6,   1, 6, 2,
		// 뒷면
		5, 4, 7,   5, 7, 6,
		// 왼쪽면
		4, 0, 3,   4, 3, 7,
		// 윗면
		4, 5, 1,   4, 1, 0,
		// 아랫면
		3, 2, 6,   3, 6, 7
	};
}

FloorMesh::FloorMesh(float fWidth, float fDepth)
{
	float w = fWidth * 0.5f;
	float d = fDepth * 0.5f;

	m_Vertices.push_back(CVertex(-w, -2.0f, +d)); // 왼쪽 앞
	m_Vertices.push_back(CVertex(+w, -2.0f, +d)); // 오른쪽 앞
	m_Vertices.push_back(CVertex(+w, -2.0f, -d)); // 오른쪽 뒤
	m_Vertices.push_back(CVertex(-w, -2.0f, -d)); // 왼쪽 뒤

	m_Indices = {
		0, 1, 2,  0, 2, 3
	};
}