#pragma once

#include"Mesh.h"

////////////////////////////////////////////////////////////////////////////////

class CModel
{
public:
	CModel() {};
	CModel(char *pFileName);
	virtual ~CModel();

protected:
	CMesh			*m_pMesh = NULL;
	CCubeMesh		*m_pCubeMesh = NULL;

	int				m_nMaterials;

	bool			m_bHasAnimation = false;

public:
	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh) { m_pMesh = pMesh; m_pCubeMesh = pCubeMesh; }
	void UpdateCollisionBox(BoundingBox &xmAABB, XMFLOAT4X4 &xmf4x4World);

protected:
	char			*m_pstrName = NULL;

public:
	bool IsName(char *pstrName) { if (!strcmp(m_pstrName, pstrName))	return true; return false; };
};