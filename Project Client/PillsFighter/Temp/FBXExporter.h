#pragma once
#include "Utilities.h"
#include <unordered_map>
#include "Material.h"

class FBXExporter
{
public:
	FBXExporter();
	~FBXExporter();
	bool Initialize();
	bool LoadScene(const char *pstrFileName);

	void ExportFBX();

	void WriteMeshToStream(XMFLOAT3* pPositions, XMFLOAT3* pNormals, XMFLOAT2* pTextureCoords0, UINT* pnIndices);

	void CleanupFbxManager();

	int GetIndices() { return mTriangleCount * 3; }
	int GetVertices() { return mVertices.size(); }

private:
	FbxManager * mFBXManager;
	FbxScene* mFBXScene;
	const char* mInputFilePath;

	std::unordered_map<unsigned int, CtrlPoint*> mControlPoints;
	unsigned int mTriangleCount;
	std::vector<Triangle> mTriangles;
	std::vector<PNTIWVertex> mVertices;

	bool mHasAnimation;
	Skeleton mSkeleton;
	FbxLongLong mAnimationLength;
	std::string mAnimationName;

	std::unordered_map<unsigned int, Material*> mMaterialLookUp;

	LARGE_INTEGER mCPUFreq;
	LARGE_INTEGER mStart;
	LARGE_INTEGER mEnd;

private:
	void ProcessGeometry(FbxNode* pfbxNode);
	void ProcessSkeletonHierarchy(FbxNode* inRootNode);
	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);
	void ProcessControlPoints(FbxNode* inNode);
	void ProcessJointsAndAnimations(FbxNode* inNode);
	unsigned int FindJointIndexUsingName(const std::string& inJointName);
	void ProcessMesh(FbxNode* inNode);

	void ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal);
	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal);
	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent);

	void Optimize();
	int FindVertex(const PNTIWVertex& inTargetVertex, const std::vector<PNTIWVertex>& uniqueVertices);

	void AssociateMaterialToMesh(FbxNode* inNode);
	void ProcessMaterials(FbxNode* inNode);
	void ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, unsigned int inMaterialIndex);
	void ProcessMaterialTexture(FbxSurfaceMaterial* inMaterial, Material* ioMaterial);
	void PrintMaterial();
	void PrintTriangles();

	void WriteAnimationToStream(std::ostream& inStream);
};