
float3 Rotate(float3 v, float fPitch, float fYaw, float fRoll)
{
	float3 vReturn = v;

	if (abs(fPitch) > 0.01f)
	{
		float fRadianX = radians(fPitch);

		float3x3 f3x3XRotate = {
			1, 0, 0,
			0, cos(fRadianX), sin(fRadianX),
			0, -sin(fRadianX), cos(fRadianX)
		};

		vReturn = mul(vReturn, f3x3XRotate);
	}

	if (abs(fYaw) > 0.01f)
	{
		float fRadianY = radians(fYaw);

		float3x3 f3x3YRotate = {
			cos(fRadianY), 0, -sin(fRadianY),
			0, 1, 0,
			sin(fRadianY), 0, cos(fRadianY)
		};

		vReturn = mul(vReturn, f3x3YRotate);
	}

	if (abs(fRoll) > 0.01f)
	{
		float fRadianZ = radians(fRoll);

		float3x3 f3x3ZRotate = {
			cos(fRadianZ), sin(fRadianZ), 0,
			-sin(fRadianZ), cos(fRadianZ), 0,
			0, 0, 1
		};

		vReturn = mul(vReturn, f3x3ZRotate);
	}

	return vReturn;
}

float3x3 RotateAxis(float3 vAxis, float fAngle)
{
	float fTheta = radians(fAngle);

	float3 v0 = float3(
		cos(fTheta) + (1.0f - cos(fTheta)) * vAxis.x * vAxis.x,
		(1.0f - cos(fTheta)) * vAxis.x * vAxis.y - sin(fTheta) * vAxis.z,
		(1.0f - cos(fTheta)) * vAxis.x * vAxis.z + sin(fTheta) * vAxis.y);

	float3 v1 = float3(
		(1.0f - cos(fTheta)) * vAxis.x * vAxis.y + sin(fTheta) * vAxis.z,
		cos(fTheta) + (1.0f - cos(fTheta)) * vAxis.y *  vAxis.y,
		(1.0f - cos(fTheta)) * vAxis.y * vAxis.z - sin(fTheta) * vAxis.x
		);

	float3 v2 = float3(
		(1.0f - cos(fTheta)) * vAxis.x * vAxis.z - sin(fTheta) * vAxis.y,
		(1.0f - cos(fTheta)) * vAxis.y * vAxis.z + sin(fTheta) * vAxis.x,
		cos(fTheta) + (1.0f - cos(fTheta)) * vAxis.z * vAxis.z
		);

	return float3x3(v0, v1, v2);
}
