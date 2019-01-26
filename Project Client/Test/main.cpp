#include<iostream>
#include<fbxsdk.h>
#include<unordered_map>
#pragma comment(lib, "libfbxsdk-md.lib")

std::string GetFileName(const std::string strFileName)
{
	std::string strPathSeparator("\\");
	std::string strSuffixSeparator(".");

	int nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	}

	int nSuffixpos = strFileName.find_last_of(strSuffixSeparator);

	return strFileName.substr(nPathpos, nSuffixpos - nPathpos);
}

std::string GetFilePath(const std::string strFileName)
{
	std::string strPathSeparator("\\");

	int nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	}
	std::cout << nPathpos << std::endl;

	return strFileName.substr(0, nPathpos);
}

FbxNode* GetNode(FbxNode* pfbxNode, FbxNodeAttribute::EType fbxNodeType)
{
	auto fbxNodeAttribute = pfbxNode->GetNodeAttribute();

	if (fbxNodeAttribute)
	{
		if (fbxNodeAttribute->GetAttributeType() == fbxNodeType)
		{
			return pfbxNode;
		}
	}

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
	{
		auto pNode = GetNode(pfbxNode->GetChild(i), fbxNodeType);

		if (pNode)
			return pNode;
	}

	return NULL;
}

class CFloat2
{
public:
	CFloat2() { m_x = 0.0f; m_y = 0.0f; };
	CFloat2(float x, float y) :m_x(x), m_y(y) {};
	virtual ~CFloat2() {};
	bool operator==(const CFloat2 rhs)
	{
		return (m_x == rhs.m_x) && (m_y == rhs.m_y);
	}

	float m_x, m_y;
	void PrintInfo(const char *pstrFirst) { printf("%s [%f, %f]\n", pstrFirst, m_x, m_y); }
};

class CFloat3
{
public:
	CFloat3() { m_x = 0.0f; m_y = 0.0f; m_z = 0.0f;};
	CFloat3(float x, float y, float z) :m_x(x), m_y(y), m_z(z) {};
	virtual ~CFloat3() {};
	bool operator==(const CFloat3 rhs)
	{
		return (m_x == rhs.m_x) && (m_y == rhs.m_y) && (m_z == rhs.m_z);
	}

	float m_x, m_y, m_z;
	void PrintInfo(const char *pstrFirst) { printf("%s [%f, %f, %f]\n", pstrFirst, m_x, m_y, m_z); }
};

class CFloat4
{
public:
	CFloat4() {};
	CFloat4(float x, float y, float z, float w) :m_x(x), m_y(y), m_z(z), m_w(w) {};
	virtual ~CFloat4() {};

	float m_x, m_y, m_z, m_w;
	void PrintInfo() { printf("%f, %f, %f, %f", m_x, m_y, m_z, m_w); }
};

class CMesh
{
public:
	CMesh() {};
	virtual ~CMesh()
	{
		if (m_pf3Positions) delete[] m_pf3Positions;
		if (m_pf3Normals) delete[] m_pf3Normals;
		if (m_pf3BiNormals) delete[] m_pf3BiNormals;
		if (m_pf3Tangents) delete[] m_pf3Tangents;
		if (m_pf2UVs) delete[] m_pf2UVs;
		if (m_pnMaterialIndex) delete[] m_pnMaterialIndex;
		if (m_pnIndices) delete[] m_pnIndices;
	};

	int			m_nVetices = 0;
	CFloat3		*m_pf3Positions = NULL;
	CFloat3		*m_pf3Normals = NULL;
	CFloat3		*m_pf3BiNormals = NULL;
	CFloat3		*m_pf3Tangents = NULL;
	CFloat2		*m_pf2UVs = NULL;
	int			*m_pnMaterialIndex = NULL;
	int			*m_pnIndices = NULL;

	int			**m_ppnBoneIndics;
	float		**m_ppfBoneWeights;

	void AddBoneIndex(int nIndex, float fWeight)
	{
		for (int i = 0; i < m_nVetices; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (m_ppnBoneIndics[i][j] == NULL)
				{
					m_ppnBoneIndics[i][j] = nIndex;
					m_ppfBoneWeights[i][j] = fWeight;
					break;
				}
			}
		}
	}
};

CMesh* CreateMeshInfo(FbxNode *pfbxMeshNode)
{
	auto pfbxNodeAttribute = pfbxMeshNode->GetNodeAttribute();
	auto fbxAttributeType = pfbxNodeAttribute->GetAttributeType();

	FbxMesh *pFbxMesh = pfbxMeshNode->GetMesh();
	printf("Control Point Count : %d\n", pFbxMesh->GetControlPointsCount());
	int nPolygon = pFbxMesh->GetPolygonCount();
	int nVertices = nPolygon * 3;

	CMesh *pMyMesh = new CMesh();
	printf("Vertex Count : %d\n", nVertices);
	pMyMesh->m_nVetices = nVertices;
	pMyMesh->m_pf3Positions = new CFloat3[nVertices];
	pMyMesh->m_pf3Normals = new CFloat3[nVertices];
	pMyMesh->m_pf3BiNormals = new CFloat3[nVertices];
	pMyMesh->m_pf3Tangents = new CFloat3[nVertices];
	pMyMesh->m_pf2UVs = new CFloat2[nVertices];
	pMyMesh->m_pnMaterialIndex = new int[nVertices];

	pMyMesh->m_pnIndices = new int[nVertices];
	pMyMesh->m_ppnBoneIndics = new int*[nVertices];
	pMyMesh->m_ppfBoneWeights = new float*[nVertices];
	int nIndex = 0;
	
	for (int i = 0; i < nPolygon; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			pMyMesh->m_ppnBoneIndics[i * 3 + j] = new int[4];
			memset(pMyMesh->m_ppnBoneIndics[i * 3 + j], 0, sizeof(int) * 4);

			pMyMesh->m_ppfBoneWeights[i * 3 + j] = new float[4];
			memset(pMyMesh->m_ppfBoneWeights[i * 3 + j], 0, sizeof(float) * 4);

			FbxGeometryElementNormal *pFbxGeoElemNorm = pFbxMesh->GetElementNormal();
			FbxGeometryElementUV *pFbxGeoElemUV = pFbxMesh->GetElementUV();
			FbxGeometryElementBinormal *pFbxGeoElemBinormal = pFbxMesh->GetElementBinormal();
			FbxGeometryElementTangent *pFbxGeoElemTangent = pFbxMesh->GetElementTangent();

			int nCtrlPointIndex = pFbxMesh->GetPolygonVertex(i, j);
			int nTextureUVIndex = pFbxMesh->GetTextureUVIndex(i, j);
			int nIndexByCtrlPoint = pFbxGeoElemNorm->GetIndexArray().GetAt(nCtrlPointIndex);
			int nIndexByIndex = pFbxGeoElemNorm->GetIndexArray().GetAt(nIndex);

			pMyMesh->m_pnIndices[i * 3 + j] = nCtrlPointIndex;

			FbxVector4 fv4CtrlPoint = pFbxMesh->GetControlPointAt(nCtrlPointIndex);
			pMyMesh->m_pf3Positions[i * 3 + j].m_x = static_cast<float>(fv4CtrlPoint.mData[0]);
			pMyMesh->m_pf3Positions[i * 3 + j].m_y = static_cast<float>(fv4CtrlPoint.mData[1]);
			pMyMesh->m_pf3Positions[i * 3 + j].m_z = static_cast<float>(fv4CtrlPoint.mData[2]);

			auto fbxElemRefMode = pFbxGeoElemNorm->GetReferenceMode();

			switch (pFbxGeoElemNorm->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (fbxElemRefMode)
				{
				case FbxGeometryElement::eDirect:
				{
					if (pFbxGeoElemNorm)
					{
						pMyMesh->m_pf3Normals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);
					}

					if (pFbxGeoElemBinormal)
					{
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);
					}

					if (pFbxGeoElemTangent)
					{
						pMyMesh->m_pf3Tangents[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);
					}

					if (pFbxGeoElemUV)
					{
						pMyMesh->m_pf2UVs[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]);
						pMyMesh->m_pf2UVs[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]);
					}
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					if (pFbxGeoElemNorm)
					{
						pMyMesh->m_pf3Normals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);
					}

					if (pFbxGeoElemBinormal)
					{
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);
					}

					if (pFbxGeoElemTangent)
					{
						pMyMesh->m_pf3Tangents[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);
					}

					if (pFbxGeoElemUV)
					{
						pMyMesh->m_pf2UVs[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]);
						pMyMesh->m_pf2UVs[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]);
					}
				}
				break;
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (fbxElemRefMode)
				{
				case FbxGeometryElement::eDirect:
				{
					if (pFbxGeoElemNorm)
					{
						pMyMesh->m_pf3Normals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[0]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[1]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[2]);
					}

					if (pFbxGeoElemBinormal)
					{
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[0]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[1]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[2]);
					}

					if (pFbxGeoElemTangent)
					{
						pMyMesh->m_pf3Tangents[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[0]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[1]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[2]);
					}

					if (pFbxGeoElemUV)
					{
						pMyMesh->m_pf2UVs[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0]);
						pMyMesh->m_pf2UVs[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]);
					}
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					if (pFbxGeoElemNorm)
					{
						pMyMesh->m_pf3Normals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[0]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[1]);
						pMyMesh->m_pf3Normals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[2]);
					}

					if (pFbxGeoElemBinormal)
					{
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[0]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[1]);
						pMyMesh->m_pf3BiNormals[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[2]);
					}

					if (pFbxGeoElemTangent)
					{
						pMyMesh->m_pf3Tangents[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[0]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[1]);
						pMyMesh->m_pf3Tangents[i * 3 + j].m_z = static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[2]);
					}

					if (pFbxGeoElemUV)
					{
						pMyMesh->m_pf2UVs[i * 3 + j].m_x = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0]);
						pMyMesh->m_pf2UVs[i * 3 + j].m_y = static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]);
					}
				}
				break;
				}
				break;
			}

			auto fbxMaterialIndex = &(pFbxMesh->GetElementMaterial()->GetIndexArray());
			auto fbxMaterialMappingMode = pFbxMesh->GetElementMaterial()->GetMappingMode();

			switch (fbxMaterialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				pMyMesh->m_pnMaterialIndex[i * 3 + j] = fbxMaterialIndex->GetAt(i);
			}
			break;
			case FbxGeometryElement::eAllSame:
			{
				pMyMesh->m_pnMaterialIndex[i * 3 + j] = fbxMaterialIndex->GetAt(0);
			}
			break;
			}

			nIndex++;
			//printf("\n");
		}
		printf("\n");
	}

	/*int nMaterialCount = pfbxMeshNode->GetMaterialCount();

	for (int i = 0; i < nMaterialCount; i++)
	{
		auto pfbxMaterial = pfbxMeshNode->GetMaterial(i);
		std::cout << "Material Count : " << i << "\n";

		if (pfbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			std::cout << "Is Phong\n";

			FbxDouble3 fbxd3Ambient = ((FbxSurfacePhong*)pfbxMaterial)->Ambient;
			printf("Ambient Color : %f, %f, %f\n",
				fbxd3Ambient.mData[0],
				fbxd3Ambient.mData[1],
				fbxd3Ambient.mData[2]);

			FbxDouble3 fbxd3Diffuse = ((FbxSurfacePhong*)pfbxMaterial)->Diffuse;
			printf("Diffuse Color : %f, %f, %f\n",
				fbxd3Diffuse.mData[0],
				fbxd3Diffuse.mData[1],
				fbxd3Diffuse.mData[2]);

			FbxDouble3 fbxd3Specular = ((FbxSurfacePhong*)pfbxMaterial)->Specular;
			printf("Specular Color : %f, %f, %f\n",
				fbxd3Specular.mData[0],
				fbxd3Specular.mData[1],
				fbxd3Specular.mData[2]);

			FbxDouble3 fbxd3Emissive = ((FbxSurfacePhong*)pfbxMaterial)->Emissive;
			printf("Emissive Color : %f, %f, %f\n",
				fbxd3Emissive.mData[0],
				fbxd3Emissive.mData[1],
				fbxd3Emissive.mData[2]);

			FbxDouble3 fbxd3Reflection = ((FbxSurfacePhong*)pfbxMaterial)->Reflection;
			printf("Reflection Color : %f, %f, %f\n",
				fbxd3Reflection.mData[0],
				fbxd3Reflection.mData[1],
				fbxd3Reflection.mData[2]);

			FbxDouble fbxd3TransparencyFactor = ((FbxSurfacePhong*)pfbxMaterial)->TransparencyFactor;
			printf("Transparency Factor : %f\n",
				fbxd3TransparencyFactor);

			FbxDouble fbxd3Shininess = ((FbxSurfacePhong*)pfbxMaterial)->Shininess;
			printf("Shininess : %f\n",
				fbxd3Shininess);

			FbxDouble fbxd3SpecularFactor = ((FbxSurfacePhong*)pfbxMaterial)->SpecularFactor;
			printf("Specular Factor : %f\n",
				fbxd3SpecularFactor);

			FbxDouble fbxd3ReflectionFactor = ((FbxSurfacePhong*)pfbxMaterial)->ReflectionFactor;
			printf("Reflection Factor : %f\n",
				fbxd3ReflectionFactor);
		}
		else if (pfbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			std::cout << "Is Lambert\n";

			FbxDouble3 fbxd3Ambient = ((FbxSurfaceLambert*)pfbxMaterial)->Ambient;
			printf("Ambient Color : %f, %f, %f\n",
				fbxd3Ambient.mData[0],
				fbxd3Ambient.mData[1],
				fbxd3Ambient.mData[2]);

			FbxDouble3 fbxd3Diffuse = ((FbxSurfaceLambert*)pfbxMaterial)->Diffuse;
			printf("Diffuse Color : %f, %f, %f\n",
				fbxd3Diffuse.mData[0],
				fbxd3Diffuse.mData[1],
				fbxd3Diffuse.mData[2]);

			FbxDouble3 fbxd3Emissive = ((FbxSurfaceLambert*)pfbxMaterial)->Emissive;
			printf("Emissive Color : %f, %f, %f\n",
				fbxd3Emissive.mData[0],
				fbxd3Emissive.mData[1],
				fbxd3Emissive.mData[2]);

			FbxDouble fbxd3TransparencyFactor = ((FbxSurfaceLambert*)pfbxMaterial)->TransparencyFactor;
			printf("Transparency Factor : %f\n",
				fbxd3TransparencyFactor);
		}

		int j;

		FBXSDK_FOR_EACH_TEXTURE(j)
		{

			auto fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);

			if (fbxProperty.IsValid())
			{
				std::cout << "Layer : " << j << "\n";

				FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(j);
				if (pfbxLayeredTexture)
				{
					std::cout << "Current Material is Layered\n";
				}
				else
				{
					FbxTexture *pfbxTexture = fbxProperty.GetSrcObject<FbxTexture>(j);
					if (pfbxTexture)
					{
						FbxFileTexture *pFbxFileTexture = FbxCast<FbxFileTexture>(pfbxTexture);
						std::string strFileName = GetFileName(pFbxFileTexture->GetFileName());

						if (pFbxFileTexture)
						{
							std::string strTextureType = fbxProperty.GetNameAsCStr();

							if (strTextureType == "DiffuseColor")
							{
								printf("Is Diffuse Map : %s\n", strFileName.c_str());
							}
							else if (strTextureType == "SpecularColor")
							{
								printf("Is Specular Map : %s\n", strFileName.c_str());
							}
							else if (strTextureType == "Bump")
							{
								printf("Is Bump Map : %s\n", strFileName.c_str());
							}
						}
					}
				}
			}
		}

		std::cout << "\n";
	}*/

	return pMyMesh;
}

CMesh* NewCreateMeshInfo(FbxNode *pfbxMeshNode)
{
	auto pfbxNodeAttribute = pfbxMeshNode->GetNodeAttribute();
	auto fbxAttributeType = pfbxNodeAttribute->GetAttributeType();

	FbxMesh *pFbxMesh = pfbxMeshNode->GetMesh();
	int nControlPoint = pFbxMesh->GetControlPointsCount();
	int nPolygon = pFbxMesh->GetPolygonCount();
	int nVertices = nPolygon * 3;

	CMesh *pMyMesh = new CMesh();
	printf("Vertex Count : %d\n", nControlPoint);
	pMyMesh->m_nVetices = nControlPoint;
	pMyMesh->m_pf3Positions = new CFloat3[nControlPoint];
	pMyMesh->m_pf3Normals = new CFloat3[nControlPoint];
	pMyMesh->m_pf2UVs = new CFloat2[nControlPoint];
	pMyMesh->m_pnMaterialIndex = new int[nControlPoint];

	pMyMesh->m_pnIndices = new int[nVertices];
	//pMyMesh->m_ppnBoneIndics = new int*[nVertices];
	//pMyMesh->m_ppfBoneWeights = new float*[nVertices];

	std::unordered_map<int, CFloat3> umPositions;
	std::unordered_map<int, CFloat3> umNormals;
	std::unordered_map<int, CFloat3> umBiNormals;
	std::unordered_map<int, CFloat3> umTangents;
	std::unordered_map<int, CFloat2> umUVs;
	std::unordered_map<int, int> umMaterialsIndex;

	umPositions.reserve(nControlPoint);
	umNormals.reserve(nControlPoint);
	umBiNormals.reserve(nControlPoint);
	umTangents.reserve(nControlPoint);
	umUVs.reserve(nControlPoint);
	umMaterialsIndex.reserve(nControlPoint);

	FbxGeometryElementNormal *pFbxGeoElemNorm = pFbxMesh->GetElementNormal();
	FbxVector4 fbxv4Normal;

	FbxGeometryElementUV *pFbxGeoElemUV = pFbxMesh->GetElementUV();
	FbxVector2 fbxv2UV;

	FbxGeometryElementBinormal *pFbxGeoElemBinormal = pFbxMesh->GetElementBinormal();
	FbxVector4 fbxv4Binormal;
	if(pFbxGeoElemBinormal)
		pMyMesh->m_pf3BiNormals = new CFloat3[nControlPoint];

	FbxGeometryElementTangent *pFbxGeoElemTangent = pFbxMesh->GetElementTangent();
	FbxVector4 fbxv4Tangent;
	if(pFbxGeoElemTangent)
		pMyMesh->m_pf3Tangents = new CFloat3[nControlPoint];

	for (int i = 0; i < nPolygon; i++)
	{
		for (int j = 0; j < 3; j++)
		{


			int nCtrlPointIndex = pFbxMesh->GetPolygonVertex(i, j);
			int nTextureUVIndex = pFbxMesh->GetTextureUVIndex(i, j);

			pMyMesh->m_pnIndices[i * 3 + j] = nCtrlPointIndex;

			FbxVector4 fv4CtrlPointPosition = pFbxMesh->GetControlPointAt(nCtrlPointIndex);
			umPositions[nCtrlPointIndex].m_x = static_cast<float>(fv4CtrlPointPosition.mData[0]);
			umPositions[nCtrlPointIndex].m_y = static_cast<float>(fv4CtrlPointPosition.mData[1]);
			umPositions[nCtrlPointIndex].m_z = static_cast<float>(fv4CtrlPointPosition.mData[2]);

			auto fbxElemRefMode = pFbxGeoElemNorm->GetReferenceMode();

			switch (pFbxGeoElemNorm->GetMappingMode())
			{
			case FbxGeometryElement::eByPolygonVertex:
				switch (fbxElemRefMode)
				{
				case FbxGeometryElement::eDirect:
				{
					if (pFbxGeoElemNorm)
					{
						fbxv4Normal = pFbxGeoElemNorm->GetDirectArray().GetAt(i * 3 + j);
					}

					if (pFbxGeoElemBinormal)
					{
						fbxv4Binormal = pFbxGeoElemBinormal->GetDirectArray().GetAt(i * 3 + j);
					}

					if (pFbxGeoElemTangent)
					{
						fbxv4Tangent = pFbxGeoElemTangent->GetDirectArray().GetAt(i * 3 + j);
					}

					if (pFbxGeoElemUV)
					{
						fbxv2UV = pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex);
					}


				}
				break;
				}
				break;
			}

			umNormals[nCtrlPointIndex].m_x += static_cast<float>(fbxv4Normal.mData[0]);
			umNormals[nCtrlPointIndex].m_x /= 2.0f;

			umNormals[nCtrlPointIndex].m_y += static_cast<float>(fbxv4Normal.mData[1]);
			umNormals[nCtrlPointIndex].m_y /= 2.0f;

			umNormals[nCtrlPointIndex].m_z += static_cast<float>(fbxv4Normal.mData[2]);
			umNormals[nCtrlPointIndex].m_z /= 2.0f;

			//

			umBiNormals[nCtrlPointIndex].m_x += static_cast<float>(fbxv4Binormal.mData[0]);
			umBiNormals[nCtrlPointIndex].m_x /= 2.0f;

			umBiNormals[nCtrlPointIndex].m_y += static_cast<float>(fbxv4Binormal.mData[1]);
			umBiNormals[nCtrlPointIndex].m_y /= 2.0f;

			umBiNormals[nCtrlPointIndex].m_z += static_cast<float>(fbxv4Binormal.mData[2]);
			umBiNormals[nCtrlPointIndex].m_z /= 2.0f;

			//

			umTangents[nCtrlPointIndex].m_x += static_cast<float>(fbxv4Tangent.mData[0]);
			umTangents[nCtrlPointIndex].m_x /= 2.0f;

			umTangents[nCtrlPointIndex].m_y += static_cast<float>(fbxv4Tangent.mData[1]);
			umTangents[nCtrlPointIndex].m_y /= 2.0f;

			umTangents[nCtrlPointIndex].m_z += static_cast<float>(fbxv4Tangent.mData[2]);
			umTangents[nCtrlPointIndex].m_z /= 2.0f;

			//

			umUVs[nCtrlPointIndex].m_x += static_cast<float>(fbxv2UV.mData[0]);
			umUVs[nCtrlPointIndex].m_x /= 2.0f;

			umUVs[nCtrlPointIndex].m_y += static_cast<float>(fbxv2UV.mData[1]);
			umUVs[nCtrlPointIndex].m_y /= 2.0f;

			auto fbxMaterialIndex = &(pFbxMesh->GetElementMaterial()->GetIndexArray());
			auto fbxMaterialMappingMode = pFbxMesh->GetElementMaterial()->GetMappingMode();

			switch (fbxMaterialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				umMaterialsIndex[nCtrlPointIndex] = fbxMaterialIndex->GetAt(i);
			}
			break;
			case FbxGeometryElement::eAllSame:
			{
				umMaterialsIndex[nCtrlPointIndex] = fbxMaterialIndex->GetAt(0);
			}
			break;
			}
		}
	}

	//int nMaterialCount = pfbxMeshNode->GetMaterialCount();

	//for (int i = 0; i < nMaterialCount; i++)
	//{
	//	auto pfbxMaterial = pfbxMeshNode->GetMaterial(i);
	//	std::cout << "Material Count : " << i << "\n";

	//	if (pfbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	//	{
	//		std::cout << "Is Phong\n";

	//		FbxDouble3 fbxd3Ambient = ((FbxSurfacePhong*)pfbxMaterial)->Ambient;
	//		printf("Ambient Color : %f, %f, %f\n",
	//			fbxd3Ambient.mData[0],
	//			fbxd3Ambient.mData[1],
	//			fbxd3Ambient.mData[2]);

	//		FbxDouble3 fbxd3Diffuse = ((FbxSurfacePhong*)pfbxMaterial)->Diffuse;
	//		printf("Diffuse Color : %f, %f, %f\n",
	//			fbxd3Diffuse.mData[0],
	//			fbxd3Diffuse.mData[1],
	//			fbxd3Diffuse.mData[2]);

	//		FbxDouble3 fbxd3Specular = ((FbxSurfacePhong*)pfbxMaterial)->Specular;
	//		printf("Specular Color : %f, %f, %f\n",
	//			fbxd3Specular.mData[0],
	//			fbxd3Specular.mData[1],
	//			fbxd3Specular.mData[2]);

	//		FbxDouble3 fbxd3Emissive = ((FbxSurfacePhong*)pfbxMaterial)->Emissive;
	//		printf("Emissive Color : %f, %f, %f\n",
	//			fbxd3Emissive.mData[0],
	//			fbxd3Emissive.mData[1],
	//			fbxd3Emissive.mData[2]);

	//		FbxDouble3 fbxd3Reflection = ((FbxSurfacePhong*)pfbxMaterial)->Reflection;
	//		printf("Reflection Color : %f, %f, %f\n",
	//			fbxd3Reflection.mData[0],
	//			fbxd3Reflection.mData[1],
	//			fbxd3Reflection.mData[2]);

	//		FbxDouble fbxd3TransparencyFactor = ((FbxSurfacePhong*)pfbxMaterial)->TransparencyFactor;
	//		printf("Transparency Factor : %f\n",
	//			fbxd3TransparencyFactor);

	//		FbxDouble fbxd3Shininess = ((FbxSurfacePhong*)pfbxMaterial)->Shininess;
	//		printf("Shininess : %f\n",
	//			fbxd3Shininess);

	//		FbxDouble fbxd3SpecularFactor = ((FbxSurfacePhong*)pfbxMaterial)->SpecularFactor;
	//		printf("Specular Factor : %f\n",
	//			fbxd3SpecularFactor);

	//		FbxDouble fbxd3ReflectionFactor = ((FbxSurfacePhong*)pfbxMaterial)->ReflectionFactor;
	//		printf("Reflection Factor : %f\n",
	//			fbxd3ReflectionFactor);
	//	}
	//	else if (pfbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	//	{
	//		std::cout << "Is Lambert\n";

	//		FbxDouble3 fbxd3Ambient = ((FbxSurfaceLambert*)pfbxMaterial)->Ambient;
	//		printf("Ambient Color : %f, %f, %f\n",
	//			fbxd3Ambient.mData[0],
	//			fbxd3Ambient.mData[1],
	//			fbxd3Ambient.mData[2]);

	//		FbxDouble3 fbxd3Diffuse = ((FbxSurfaceLambert*)pfbxMaterial)->Diffuse;
	//		printf("Diffuse Color : %f, %f, %f\n",
	//			fbxd3Diffuse.mData[0],
	//			fbxd3Diffuse.mData[1],
	//			fbxd3Diffuse.mData[2]);

	//		FbxDouble3 fbxd3Emissive = ((FbxSurfaceLambert*)pfbxMaterial)->Emissive;
	//		printf("Emissive Color : %f, %f, %f\n",
	//			fbxd3Emissive.mData[0],
	//			fbxd3Emissive.mData[1],
	//			fbxd3Emissive.mData[2]);

	//		FbxDouble fbxd3TransparencyFactor = ((FbxSurfaceLambert*)pfbxMaterial)->TransparencyFactor;
	//		printf("Transparency Factor : %f\n",
	//			fbxd3TransparencyFactor);
	//	}

	//	int j;

	//	FBXSDK_FOR_EACH_TEXTURE(j)
	//	{

	//		auto fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);

	//		if (fbxProperty.IsValid())
	//		{
	//			std::cout << "Layer : " << j << "\n";

	//			FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(j);
	//			if (pfbxLayeredTexture)
	//			{
	//				std::cout << "Current Material is Layered\n";
	//			}
	//			else
	//			{
	//				FbxTexture *pfbxTexture = fbxProperty.GetSrcObject<FbxTexture>(j);
	//				if (pfbxTexture)
	//				{
	//					FbxFileTexture *pFbxFileTexture = FbxCast<FbxFileTexture>(pfbxTexture);
	//					std::string strFileName = GetFileName(pFbxFileTexture->GetFileName());

	//					if (pFbxFileTexture)
	//					{
	//						std::string strTextureType = fbxProperty.GetNameAsCStr();

	//						if (strTextureType == "DiffuseColor")
	//						{
	//							printf("Is Diffuse Map : %s\n", strFileName.c_str());
	//						}
	//						else if (strTextureType == "SpecularColor")
	//						{
	//							printf("Is Specular Map : %s\n", strFileName.c_str());
	//						}
	//						else if (strTextureType == "Bump")
	//						{
	//							printf("Is Bump Map : %s\n", strFileName.c_str());
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}

	//	std::cout << "\n";
	//}

	for (int i = 0; i < nControlPoint; i++)
	{
		pMyMesh->m_pf3Positions[i] = umPositions[i];
		pMyMesh->m_pf2UVs[i] = umUVs[i];
		pMyMesh->m_pf3Normals[i] = umNormals[i];
		pMyMesh->m_pnMaterialIndex[i] = umMaterialsIndex[i];

		if (pFbxGeoElemBinormal)
			pMyMesh->m_pf3BiNormals[i] = umBiNormals[i];

		if (pFbxGeoElemTangent)
			pMyMesh->m_pf3Tangents[i] = umTangents[i];
	}

	return pMyMesh;
}

class Float4x4
{
public:
	Float4x4() {};
	Float4x4(FbxAMatrix fbxMatrix)
	{
		m_11 = static_cast<float>(fbxMatrix.mData[0].mData[0]);
		m_12 = static_cast<float>(fbxMatrix.mData[0].mData[1]);
		m_13 = static_cast<float>(fbxMatrix.mData[0].mData[2]);
		m_14 = static_cast<float>(fbxMatrix.mData[0].mData[3]);

		m_21 = static_cast<float>(fbxMatrix.mData[1].mData[0]);
		m_22 = static_cast<float>(fbxMatrix.mData[1].mData[1]);
		m_23 = static_cast<float>(fbxMatrix.mData[1].mData[2]);
		m_24 = static_cast<float>(fbxMatrix.mData[1].mData[3]);

		m_31 = static_cast<float>(fbxMatrix.mData[2].mData[0]);
		m_32 = static_cast<float>(fbxMatrix.mData[2].mData[1]);
		m_33 = static_cast<float>(fbxMatrix.mData[2].mData[2]);
		m_34 = static_cast<float>(fbxMatrix.mData[2].mData[3]);

		m_41 = static_cast<float>(fbxMatrix.mData[3].mData[0]);
		m_42 = static_cast<float>(fbxMatrix.mData[3].mData[1]);
		m_43 = static_cast<float>(fbxMatrix.mData[3].mData[2]);
		m_44 = static_cast<float>(fbxMatrix.mData[3].mData[3]);
	}
	~Float4x4() {};

	float m_11, m_12, m_13, m_14;
	float m_21, m_22, m_23, m_24;
	float m_31, m_32, m_33, m_34;
	float m_41, m_42, m_43, m_44;
};

class CSkeleton
{
public:
	CSkeleton() {};
	~CSkeleton() 
	{
		delete m_pSibling;
		delete m_pChild;
	};

	std::string strName;

	CSkeleton *m_pParent = NULL;
	CSkeleton *m_pSibling = NULL;
	CSkeleton *m_pChild = NULL;

	Float4x4  m_f4x4Binding;
	Float4x4  m_f4x4ToParent;

	void SetChild(CSkeleton *pChild)
	{
		pChild->m_pParent = this;

		if (m_pChild)
		{
			pChild->m_pSibling = m_pChild->m_pSibling;
			m_pChild->m_pSibling = pChild;
		}
		else m_pChild = pChild;
	}

	CSkeleton* GetSkeleton(const char *pstrName)
	{
		CSkeleton *pSkeleton = NULL;

		if (!strcmp(strName.c_str(), pstrName)) return this;

		if (m_pSibling)
			if (pSkeleton = m_pSibling->GetSkeleton(pstrName)) return pSkeleton;
		if (m_pChild)
			if (pSkeleton = m_pChild->GetSkeleton(pstrName)) return pSkeleton;

		return NULL;
	}
};

void GetFrame(FbxNode *pfbxNode, CSkeleton *pParent)
{
	pParent->strName = pfbxNode->GetName();

	int nChild = pfbxNode->GetChildCount();

	for (int i = 0; i < nChild; i++)
	{
		CSkeleton *pSkeleton = new CSkeleton();
		pSkeleton->m_pParent = pParent;
		pParent->SetChild(pSkeleton);

		GetFrame(pfbxNode->GetChild(i), pSkeleton);
	}
}

void PrintSkeletonInfo(CSkeleton *pSkeleton, int nTab)
{
	if (pSkeleton)
	{
		for (int i = 0; i < nTab; i++) printf("\t");
		printf("%s\n", pSkeleton->strName.c_str());
	}

	if (pSkeleton->m_pChild) PrintSkeletonInfo(pSkeleton->m_pChild, nTab+1);
	if (pSkeleton->m_pSibling) PrintSkeletonInfo(pSkeleton->m_pSibling, nTab);
}

CSkeleton* GetSkeletonHierarchy(FbxNode *pfbxNode)
{
	CSkeleton *pSkeleton = new CSkeleton();

	GetFrame(pfbxNode, pSkeleton);

	return pSkeleton;
}


void DisplayJoints(FbxNode *pfbxMeshNode, CSkeleton *pRootSkeleton, CMesh *pMyMesh)
{
	FbxMesh *pfbxMesh = pfbxMeshNode->GetMesh();

	int nDeformerCount = pfbxMesh->GetDeformerCount();
	printf("Deformer Count : %d\n", nDeformerCount);

	FbxVector4 fbxv4T = pfbxMeshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 fbxv4R = pfbxMeshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 fbxv4S = pfbxMeshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	printf("Transform : %f, %f, %f\n", static_cast<float>(fbxv4T.mData[0]), static_cast<float>(fbxv4T.mData[1]), static_cast<float>(fbxv4T.mData[2]));
	printf("Rotation : %f, %f, %f\n", static_cast<float>(fbxv4R.mData[0]), static_cast<float>(fbxv4R.mData[1]), static_cast<float>(fbxv4R.mData[2]));
	printf("Scale : %f, %f, %f\n", static_cast<float>(fbxv4S.mData[0]), static_cast<float>(fbxv4S.mData[1]), static_cast<float>(fbxv4S.mData[2]));

	for (int i = 0; i < nDeformerCount; i++)
	{
		FbxSkin *pfbxSkin = reinterpret_cast<FbxSkin*>(pfbxMesh->GetDeformer(i, FbxDeformer::eSkin));

		if (pfbxSkin)
		{
			printf("This Mesh Is Skinned\n");

			int nClusterCount = pfbxSkin->GetClusterCount();
			printf("Cluster Count : %d\n", nClusterCount);

			for (int j = 0; j < nClusterCount; j++) // Cluster = Frame
			{
				FbxCluster *pfbxCluster = pfbxSkin->GetCluster(j);
				std::string strClusterName = pfbxCluster->GetLink()->GetName();
				printf("Cluster Name : %s\n", strClusterName.c_str());

				FbxAMatrix fbxTranform;
				FbxAMatrix fbxLinkTranform;

				pfbxCluster->GetTransformMatrix(fbxTranform); // Binding Pose
				pfbxCluster->GetTransformLinkMatrix(fbxLinkTranform); // To Parent

				// Set Frame's Matrix
				CSkeleton *pSkeleton = pRootSkeleton->GetSkeleton(strClusterName.c_str());
				pSkeleton->m_f4x4Binding = Float4x4(fbxTranform);
				pSkeleton->m_f4x4ToParent = Float4x4(fbxLinkTranform);

				int nControlPointIndex = pfbxCluster->GetControlPointIndicesCount();
				int *pnIndices = pfbxCluster->GetControlPointIndices();
				double *pdWeights = pfbxCluster->GetControlPointWeights();
				printf("Control Point Index Count : %d\n", nControlPointIndex); // 영향을 받는 제어점 갯수

				for (int k = 0; k < nControlPointIndex; k++)
				{
					pMyMesh->AddBoneIndex(pnIndices[k], pdWeights[k]);
					printf("%d, %f\n", pnIndices[k], pdWeights[k]);
				}
			}
		}
	}
}

int main()
{
	FbxManager *pFbxManager = FbxManager::Create();
	pFbxManager->SetIOSettings(FbxIOSettings::Create(pFbxManager, "IOSetting"));

	FbxScene *pFbxScene = FbxScene::Create(pFbxManager, "Scene");
	FbxImporter *pFbxImporter = FbxImporter::Create(pFbxManager, "Importer");

	pFbxImporter->Initialize("./Resource/GM.fbx", -1, pFbxManager->GetIOSettings());

	if(!pFbxImporter->Import(pFbxScene))
	{
		return false;
	}
	pFbxImporter->Destroy();

	FbxNode *pFbxMeshNode = GetNode(pFbxScene->GetRootNode(), FbxNodeAttribute::eMesh);
	FbxNode *pFbxSkeletonNode = GetNode(pFbxScene->GetRootNode(), FbxNodeAttribute::eSkeleton);

	// Meshes Info
	//CMesh *pMyMesh(CreateMeshInfo(pFbxMeshNode));
	CMesh *pMyMesh(NewCreateMeshInfo(pFbxMeshNode));
	for (int i = 0; i < pMyMesh->m_nVetices; i++)
	{
		if(pMyMesh->m_pf3Positions)
			pMyMesh->m_pf3Positions[i].PrintInfo("Positions : ");
		if (pMyMesh->m_pf3Normals)
			pMyMesh->m_pf3Normals[i].PrintInfo("Normals : ");
		if (pMyMesh->m_pf3BiNormals)
			pMyMesh->m_pf3BiNormals[i].PrintInfo("BiNormals : ");
		if (pMyMesh->m_pf3Tangents)
			pMyMesh->m_pf3Tangents[i].PrintInfo("Tangents : ");
		if (pMyMesh->m_pf2UVs)
			pMyMesh->m_pf2UVs[i].PrintInfo("UVs : ");
	}


	//// Skeleton Hierarchy Info
	//CSkeleton *pSkeleton = NULL;
	//if (pFbxSkeletonNode) pSkeleton = GetSkeletonHierarchy(pFbxSkeletonNode);
	//PrintSkeletonInfo(pSkeleton, 0);

	////// Skinned Meshes Info
	//if (pFbxMeshNode) DisplayJoints(pFbxMeshNode, pSkeleton, pMyMesh);

	//delete pSkeleton;
	delete pMyMesh;

	return 0;
}