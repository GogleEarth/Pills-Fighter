#pragma once

#include"Mesh.h"

////////////////////////////////////////////////////////////////////////////////

class CModel
{
public:
	CModel();
	virtual ~CModel();

public:
	void AddRef();
	void Release();

protected:
	int				m_nReferences = 0;

protected:
	CMesh			*m_pMesh = NULL;
	CCubeMesh		*m_pCubeMesh = NULL;

	int				m_nMaterials;

	XMFLOAT4X4		m_xmf4x4ToParent;
	XMFLOAT4X4		m_xmf4x4World;

	CModel			*m_pParent = NULL;
	CModel			*m_pSibling = NULL;

	char			m_pstrModelName[64] = { 0 };
public:
	CModel			*m_pChild = NULL;

	void SetChild(CModel *pChild, bool bAddReference = false);
	void SetToParent(XMFLOAT4X4 xmf4x4ToParent) { m_xmf4x4ToParent = xmf4x4ToParent; }
	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh, bool bIsSkinned);
	XMFLOAT4X4 GetWorldTransf() { return m_xmf4x4World; }

	CMesh* GetMesh() { return m_pMesh; }


public:
	void UpdateCollisionBox(std::vector<BoundingBox>& vxmAABB, int *pnIndex);
	void CModel::UpdateWorldTransform(XMFLOAT4X4 *pxmf4x4Parent);
	void GetMeshes(int *pnStandardMeshes, int *pnSkinnedMeshes);
public:

public: // Root Model
	const char* GetFileName() { return m_pstrFileName; };
	void SetModelMeshCount(int nMeshes, int nSkinnedMeshes) { m_nMeshes = nMeshes; }
	void SetFileName(const char *pstrFileName) { m_pstrFileName = pstrFileName; }

	int GetMeshes() { return m_nMeshes; }

	XMFLOAT3 GetLook() { return XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33); }
	XMFLOAT3 GetUp() { return XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23); }
	XMFLOAT3 GetRight() { return XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13); }
	XMFLOAT3 GetPosition() { return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43); }

protected:
	// Root Model = Total
	int				m_nMeshes = 0; // All Mesh

	const char		*m_pstrFileName = NULL;

public:
	static CModel* LoadGeometryAndAnimationFromFile(char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName);
	static CModel* LoadModelFromFile(FILE *pfile, const char *pstrFileName, const char *pstrFilePath);
};