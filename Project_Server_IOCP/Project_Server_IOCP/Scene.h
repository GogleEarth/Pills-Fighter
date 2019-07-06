#pragma once

#include "Repository.h"
#include "Protocol.h"
#include "GameObject.h"

class Scene
{
protected:
	CModel*	bullet_mesh_;
	CModel*	robot_mesh_;
	std::vector<CModel*> models_;
	GameObject* Objects_[MAX_NUM_OBJECT];
	std::vector<GameObject*> Obstacles_;
	GameObject BeamsaberCollisionmesh_[24];
public:
	Scene();
	~Scene();

	virtual void BuildObjects(CRepository* pRepository) {}
	virtual void AnimateObjects(float fTimeElapsed) {}
	virtual void SceneEvent() {}

	void InsertObjectFromLoadInfFromBin(char *pstrFileName, int nGroup);

	float ReadFloatFromFile(FILE *pInFile) {
		float fValue = 0;
		UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
		return(fValue);
	}
	BYTE LeftByteFromFile(FILE *pInFile, int byte) {
		UINT nReads = 0;
		char waste[64] = { '\0' };
		nReads = (UINT)::fread(waste, sizeof(char), byte, pInFile);

		return(nReads);
	}
	BYTE ReadPosrotFromFile(FILE *pInFile, char *pstrToken) {
		BYTE nStrLength = 41;
		BYTE m_value = 7;
		UINT nReads = 0;
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), m_value, pInFile);
		pstrToken[m_value] = '\0';

		return(nReads);
	}

	void init(CRepository* pRepository);
	void InsertObject(GameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext);
	int GetIndex();
	void AddObject(GameObject* object);
	void releaseObject(int index);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GroundScene : public Scene
{
	GroundScene() {}
	~GroundScene() {}

	virtual void BuildObjects(CRepository* pRepository);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent() {}

};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SpaceScene : public Scene
{
	SpaceScene() {}
	~SpaceScene() {}

	virtual void BuildObjects(CRepository* pRepository);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent() {}

};