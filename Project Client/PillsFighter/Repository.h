#pragma once

#include"Model.h"

class CRepository
{
public:
	CRepository();
	virtual ~CRepository();

	CModel* GetModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName);
	void ReleaseUploadBuffers();

protected:
	std::vector<CModel*> m_vModels;
};

