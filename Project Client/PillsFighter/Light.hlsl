//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			4 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct LIGHT
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular;
	float3				m_vPosition;
	float 				m_fFalloff;
	float3				m_vDirection;
	float 				m_fTheta; //cos(m_fTheta)
	float3				m_vAttenuation;
	float				m_fPhi; //cos(m_fPhi)
	bool				m_bEnable;
	int 				m_nType;
	float				m_fRange;
	float				padding;
};

cbuffer cbLights : register(LIGHTS_INFO)
{
	LIGHT				gLights[MAX_LIGHTS];
	float4				gcGlobalAmbientLight;
};

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera, MATERIAL material, float fSpecularFactor)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	if (fDiffuseFactor > 0.0f)
	{
		if (fSpecularFactor != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), fSpecularFactor);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), fSpecularFactor);
#endif
		}
	}

	return((gLights[nIndex].m_cAmbient * material.m_cAmbient) +
		(gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.m_cDiffuse) +
		(gLights[nIndex].m_cSpecular * fSpecularFactor * material.m_cSpecular));
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera, MATERIAL material, float fSpecularFactor)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			//if (gMaterials[gcbGameObjectInfo.nMaterial].m_cSpecular.a != 0.0f)
			if (fSpecularFactor != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), fSpecularFactor);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), fSpecularFactor);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		//return(((gLights[nIndex].m_cAmbient * gMaterials[gcbGameObjectInfo.nMaterial].m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gcbGameObjectInfo.nMaterial].m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gcbGameObjectInfo.nMaterial].m_cSpecular)) * fAttenuationFactor);
		return(((gLights[nIndex].m_cAmbient * material.m_cAmbient) +
			(gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.m_cDiffuse) +
			(gLights[nIndex].m_cSpecular * fSpecularFactor * material.m_cSpecular)) * fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera, MATERIAL material, float fSpecularFactor)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (fSpecularFactor != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), fSpecularFactor);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), fSpecularFactor);
#endif
			}
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * material.m_cAmbient) +
			(gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.m_cDiffuse) +
			(gLights[nIndex].m_cSpecular * fSpecularFactor * material.m_cSpecular)) * fAttenuationFactor * fSpotFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 Lighting(float3 vPosition, float3 vNormal, MATERIAL material, float fSpecularFactor, float3 f3ShadowFactor)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gLights[i].m_bEnable)
		{
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += f3ShadowFactor[i] * DirectionalLight(i, vNormal, vToCamera, material, fSpecularFactor);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera, material, fSpecularFactor);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera, material, fSpecularFactor);
			}
		}
	}
	cColor += (gcGlobalAmbientLight * material.m_cAmbient);
	cColor.a = material.m_cDiffuse.a;

	//float3 diffuse = saturate(cColor);
	//diffuse = ceil(diffuse * 3) / 3.0f;
	//cColor = cColor * float4(diffuse, 1.0f);

	//return(float4(diffuse, 1.0f));
	return(cColor);
}