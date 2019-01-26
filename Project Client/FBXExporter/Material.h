#pragma once
#include "etc.h"

class Material
{
public:
	std::string mName;

	std::string mNormalMapName;

	XMFLOAT3 mAmbient;

	XMFLOAT3 mDiffuse;
	std::string mDiffuseMapName;

	XMFLOAT3 mEmissive;
	std::string mEmissiveMapName;

	XMFLOAT3 mSpecular;
	std::string mSpecularMapName;
	double mSpecularPower;

	XMFLOAT3 mReflection;
	double mReflectionFactor;

	double mShininess;

	double mTransparencyFactor;

	std::string mGlossMapName;

	virtual void WriteToStream(std::ostream& inStream)
	{
		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;
		inStream << "Specular: " << mSpecular.x << " " << mSpecular.y << " " << mSpecular.z << std::endl;
		inStream << "SpecPower: " << mSpecularPower << std::endl;
		inStream << "Reflectivity: " << mReflection.x << " " << mReflection.y << " " << mReflection.z << std::endl;

		if (!mDiffuseMapName.empty())
		{
			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
		}

		if (!mNormalMapName.empty())
		{
			inStream << "NormalMap: " << mNormalMapName << std::endl;
		}
	}
};