#pragma once
#include "Camera.h"

class CVertex
{
public:
	CVertex() { m_position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(float x, float y, float z) { m_position = XMFLOAT3(x, y, z); }
	XMFLOAT3 m_position;
};

class Mesh
{
public:
	Mesh() {}
	virtual ~Mesh() {}

private:
	int m_References = 1;

public:
	void AddRef() { m_References++; }
	void Release() { m_References--; if (m_References <= 0) delete this; }

protected:
	std::vector<CVertex> m_Vertices;
	std::vector<int> m_Indices;

public:
	virtual void Render(ID3D12GraphicsCommandList* CommandList, XMFLOAT4X4* pMtxWorld, Camera* pCamera, DWORD& color);
};

class CubeMesh : public Mesh
{
public:
	CubeMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f);
	virtual ~CubeMesh() {}
};

class FloorMesh : public Mesh
{
public:
	FloorMesh(float fWidth = 4.0f, float fDepth = 4.0f);
	virtual ~FloorMesh() {}
};