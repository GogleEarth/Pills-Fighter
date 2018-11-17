#pragma once
#include "pch.h"

class CVertex
{
protected:
	XMFLOAT3 m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }

	XMFLOAT3 GetPosition() { return m_xmf3Position; }
};

//////////////////////////////////////////////////////////////////////////////

class CIlluminatedVertex : public CVertex
{
protected:
	XMFLOAT3 m_xmf3Normal;

public:
	CIlluminatedVertex() 
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); 
		m_xmf3Normal =	XMFLOAT3(0.0f, 0.0f, 0.0f); 
	}

	CIlluminatedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal) 
	{
		m_xmf3Position = XMFLOAT3(x, y, z); 
		m_xmf3Normal = xmf3Normal;
	}

	CIlluminatedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal) 
	{
		m_xmf3Position = xmf3Position; 
		m_xmf3Normal = xmf3Normal;
	}

	~CIlluminatedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////

class CDiffusedVertex : public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse)
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf4Diffuse = xmf4Diffuse;
	}

	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse)
	{
		m_xmf3Position = xmf3Position;
		m_xmf4Diffuse = xmf4Diffuse;
	}

	~CDiffusedVertex() {}
};

class CTexturedVertex : public CIlluminatedVertex
{
public:
	XMFLOAT2	m_xmf2TexCoord;

public:
	CTexturedVertex() 
	{ 
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f); 
	}

	CTexturedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal, XMFLOAT2 xmf2TexCoord)
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf3Normal = xmf3Normal;
		m_xmf2TexCoord = xmf2TexCoord;
	}

	CTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = xmf3Position;
		m_xmf3Normal = xmf3Normal;
		m_xmf2TexCoord = xmf2TexCoord; 
	}

	~CTexturedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////

class CDiffusedTexturedVertex : public CTexturedVertex
{
public:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedTexturedVertex() 
	{ 
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	CDiffusedTexturedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf3Normal = xmf3Normal;
		m_xmf4Diffuse = xmf4Diffuse; 
		m_xmf2TexCoord = xmf2TexCoord;
	}

	CDiffusedTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = xmf3Position;
		m_xmf3Normal = xmf3Normal;
		m_xmf4Diffuse = xmf4Diffuse; 
		m_xmf2TexCoord = xmf2TexCoord; 
	}

	~CDiffusedTexturedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class CMesh
{
protected:
	UINT							m_nVertices = 0;
	CTexturedVertex					*m_pVertices = NULL;

	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다.
	인덱스 버퍼는 정점 버퍼(배열)에 대한 인덱스를 가진다.*/
	UINT							m_nIndices = 0;
	UINT							*m_pnIndices = NULL;

	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;


public:
	CMesh() {};
	CMesh(const char *pstrFileName);
	virtual ~CMesh();


	BoundingOrientedBox m_xmOOBB;
	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
	XMFLOAT3& GetExtents() { return m_xmOOBB.Extents; }
	XMFLOAT3& GetCenter() { return m_xmOOBB.Center; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(XMFLOAT3 xmf3Center, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMesh();
};