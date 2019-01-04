#include<iostream>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libfbxsdk-md.lib")

#include"fbxsdk.h"

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

FbxNode* GetMeshNode(FbxNode* pfbxNode)
{
	auto fbxNodeAttribute = pfbxNode->GetNodeAttribute();

	if (fbxNodeAttribute)
	{
		if (fbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			return pfbxNode;
		}
	}

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
	{
		auto pNode = GetMeshNode(pfbxNode->GetChild(i));

		if (pNode)
			return pNode;
	}

	return NULL;
}

int main()
{
	FbxManager *pFbxManager = FbxManager::Create();
	pFbxManager->SetIOSettings(FbxIOSettings::Create(pFbxManager, "IOSetting"));

	FbxScene *pFbxScene = FbxScene::Create(pFbxManager, "Scene");
	FbxImporter *pFbxImporter = FbxImporter::Create(pFbxManager, "Importer");

	//pFbxImporter->Initialize("./Resource/Test/testbox.fbx", -1, pFbxManager->GetIOSettings());
	pFbxImporter->Initialize("./Resource/GM/Head/Head.fbx", -1, pFbxManager->GetIOSettings());
	//pFbxImporter->Initialize("./11/Wolf.fbx", -1, pFbxManager->GetIOSettings());

	char strFilePath[256];
	const char* File = "./Resource/Test/testbox.fbx";
	strcpy(strFilePath, GetFilePath(File).c_str());

	std::cout << strFilePath << std::endl;

	if(!pFbxImporter->Import(pFbxScene))
	{
		return false;
	}
	pFbxImporter->Destroy();

	FbxNode *pFbxNode = GetMeshNode(pFbxScene->GetRootNode());
	if (!pFbxNode) return 0;

	auto pfbxNodeAttribute = pFbxNode->GetNodeAttribute();
	auto fbxAttributeType = pfbxNodeAttribute->GetAttributeType();
	
	switch (fbxAttributeType)
	{
	case FbxNodeAttribute::eMesh:
	{
		FbxMesh *pFbxMesh = pFbxNode->GetMesh();

		int nCtrlPoint = pFbxMesh->GetControlPointsCount();

		for (int i = 0; i < nCtrlPoint; i++)
		{
			FbxVector4 fv4CtrlPoint = pFbxMesh->GetControlPointAt(i);

			printf("Positions[%d] : %f, %f, %f\n",
				i,
				fv4CtrlPoint.mData[0],
				fv4CtrlPoint.mData[1],
				fv4CtrlPoint.mData[2]);
		}

		int nPolygon = pFbxMesh->GetPolygonCount();

		int nIndex = 0;

		for (int i = 0; i < nPolygon; i++)
		{
			for (int j = 0; j < 3; j++)
			{

				FbxGeometryElementNormal *pFbxGeoElemNorm = pFbxMesh->GetElementNormal();
				FbxGeometryElementUV *pFbxGeoElemUV = pFbxMesh->GetElementUV();
				FbxGeometryElementBinormal *pFbxGeoElemBinormal = pFbxMesh->GetElementBinormal();
				FbxGeometryElementTangent *pFbxGeoElemTangent = pFbxMesh->GetElementTangent();

				int nCtrlPointIndex = pFbxMesh->GetPolygonVertex(i, j);
				int nTextureUVIndex = pFbxMesh->GetTextureUVIndex(i, j);
				int nIndexByCtrlPoint = pFbxGeoElemNorm->GetIndexArray().GetAt(nCtrlPointIndex);
				int nIndexByIndex = pFbxGeoElemNorm->GetIndexArray().GetAt(nIndex);

				FbxVector4 fv4CtrlPoint = pFbxMesh->GetControlPointAt(nCtrlPointIndex);

				printf("Positions[%d] : %f, %f, %f\n",
					i,
					fv4CtrlPoint.mData[0],
					fv4CtrlPoint.mData[1],
					fv4CtrlPoint.mData[2]);

				auto fbxElemRefMode = pFbxGeoElemNorm->GetReferenceMode();

				switch (pFbxGeoElemNorm->GetMappingMode())
				{
				case FbxGeometryElement::eByControlPoint:
					switch (fbxElemRefMode)
					{
					case FbxGeometryElement::eDirect:
					{
						printf("[%d]\n", i * 3 + j);
						printf("Normals : %f, %f, %f\n", 
							pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[0],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[1],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);

						if(pFbxGeoElemBinormal)
						printf("Binormals : %f, %f, %f\n",
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[0],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[1],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);

						if (pFbxGeoElemTangent)
						printf("Tangent : %f, %f, %f\n",
							pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[0],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[1],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]);

						if (pFbxGeoElemUV)
						printf("UVs : %f, %f\n",
							pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[0],
							pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]);
					}
					break;
					case FbxGeometryElement::eIndexToDirect:
					{
						printf("[%d]\n", i * 3 + j);
						printf("Normals : %f, %f, %f\n",
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);

						if (pFbxGeoElemBinormal)
						printf("Binormals : %f, %f, %f\n",
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);

						if (pFbxGeoElemTangent)
						printf("Tangent : %f, %f, %f\n",
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]);

						if (pFbxGeoElemUV)
						printf("UVs : %f, %f\n",
							pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0],
							pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]);
					}
					break;
					}
					break;
				case FbxGeometryElement::eByPolygonVertex:
					switch (fbxElemRefMode)
					{
					case FbxGeometryElement::eDirect:
					{
						printf("[%d]\n", i * 3 + j);
						printf("Normals : %f, %f, %f\n",
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[0],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[1],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[2]);

						if (pFbxGeoElemBinormal)
						printf("Binormals : %f, %f, %f\n",
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[0],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[1],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[2]);

						if (pFbxGeoElemTangent)
						printf("Tangent : %f, %f, %f\n",
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[0],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[1],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[2]);

						if (pFbxGeoElemUV)
						printf("UVs : %f, %f\n",
							pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0],
							pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]);
					}
					break;
					case FbxGeometryElement::eIndexToDirect:
					{
						printf("[%d]\n", i * 3 + j);
						printf("Normals : %f, %f, %f\n",
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[0],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[1],
							pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[2]);

						if (pFbxGeoElemBinormal)
						printf("Binormals : %f, %f, %f\n",
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[0],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[1],
							pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[2]);

						if (pFbxGeoElemTangent)
						printf("Tangent : %f, %f, %f\n",
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[0],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[1],
							pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[2]);

						if (pFbxGeoElemUV)
						printf("UVs : %f, %f\n",
							pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0],
							pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]);
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
					printf("Material Index : %d\n", fbxMaterialIndex->GetAt(i));
				}
				break;
				case FbxGeometryElement::eAllSame:
				{
					printf("Material Index : %d\n", fbxMaterialIndex->GetAt(0));
				}
				break;
				}

				nIndex++;
				printf("\n");
			}
		}

		int nMaterialCount = pFbxNode->GetMaterialCount();

		for (int i = 0; i < nMaterialCount; i++)
		{
			auto pfbxMaterial = pFbxNode->GetMaterial(i);
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
		}

	}
	}



	return 0;
}