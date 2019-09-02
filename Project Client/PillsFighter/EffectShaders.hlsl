#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

#ifndef DEFINE_MATH
#define DEFINE_MATH
#include "Math.hlsl"
#endif

cbuffer cbSceneInfo : register(SCENE_INFO)
{
	float4		gvRandoms;
	float		gfGravity;
}

////

struct EFFECT
{
	float	m_fElapsedTime;
	float	m_fDuration;
};

ConstantBuffer<EFFECT> gEffect : register(EFFECT_INFO);

struct VS_EFFECT_INPUT
{
	float3 position : POSITION;
	float age : AGE;
	float2 size : SIZE;
	int angle : ANGLE;
	float4 color : COLOR;
};

struct VS_EFFECT_OUTPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 size : SIZE;
	int angle : ANGLE;
};

VS_EFFECT_OUTPUT VSEffectDraw(VS_EFFECT_INPUT input)
{
	VS_EFFECT_OUTPUT output;

	float fOpacity = 1.0f - smoothstep(0.0f, gEffect.m_fDuration, input.age);
	output.color = float4(input.color.rgb, fOpacity);

	output.size = input.size;
	output.position = input.position;
	output.angle = input.angle;

	return output;
}

struct GS_EFFECT_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

[maxvertexcount(4)]
void GSEffectDraw(point VS_EFFECT_OUTPUT input[1], inout TriangleStream<GS_EFFECT_OUTPUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3x3 f3x3Rotate = RotateAxis(vLook, input[0].angle);
	vUp = mul(vUp, f3x3Rotate);
	float3 vRight = normalize(cross(vUp, vLook));
	vUp = normalize(cross(vRight, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_EFFECT_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSEffectDraw(GS_EFFECT_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	return cColor * input.color;
}

VS_EFFECT_INPUT VSEffectStreamOut(VS_EFFECT_INPUT input)
{
	return input;
}

[maxvertexcount(1)]
void GSEffectStreamOut(point VS_EFFECT_INPUT input[1], inout PointStream<VS_EFFECT_INPUT> outStream)
{
	input[0].age += gEffect.m_fElapsedTime;

	if (input[0].age <= gEffect.m_fDuration)
		outStream.Append(input[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_LASER_EFFECT_INPUT
{
	float3	position : POSITION;
	float2	size : SIZE;
	float	age : AGE;
	float3	look : LOOK;
	float4	color : COLOR;
};

struct VS_LASER_EFFECT_OUTPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	float3 look : LOOK;
	float4 color : COLOR;
};

VS_LASER_EFFECT_OUTPUT VSLaserEffectDraw(VS_LASER_EFFECT_INPUT input)
{
	VS_LASER_EFFECT_OUTPUT output;

	output.size = input.size;
	output.position = input.position;
	output.look = input.look;
	output.color = input.color;

	return output;
}

struct GS_LASER_EFFECT_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};

[maxvertexcount(4)]
void GSLaserEffectDraw(point VS_LASER_EFFECT_OUTPUT input[1], inout TriangleStream<GS_LASER_EFFECT_OUTPUT> outStream)
{
	float3 vUp = input[0].look;
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3 vRight = normalize(cross(vUp, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight, 1.0f);
	fVertices[3] = float4(input[0].position + fHalfW * vRight, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 0.0f);
	fUVs[1] = float2(1.0f, 0.0f);
	fUVs[2] = float2(0.0f, 1.0f);
	fUVs[3] = float2(1.0f, 1.0f);

	GS_LASER_EFFECT_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

struct PS_EFFECT_OUTPUT
{
	float4 color : SV_TARGET0;
	float4 glow : SV_TARGET1;
};

PS_EFFECT_OUTPUT PSLaserEffectDraw(GS_LASER_EFFECT_OUTPUT input)
{
	PS_EFFECT_OUTPUT output;

	output.color = gtxtTexture[0].Sample(gssWrap, input.uv) * input.color;
	output.glow = gtxtTexture[1].Sample(gssWrap, input.uv) * input.color;

	return output;
}

VS_LASER_EFFECT_INPUT VSLaserEffectStreamOut(VS_LASER_EFFECT_INPUT input)
{
	return input;
}

[maxvertexcount(1)]
void GSLaserEffectStreamOut(point VS_LASER_EFFECT_INPUT input[1], inout PointStream<VS_LASER_EFFECT_INPUT> outStream)
{
	input[0].age += gEffect.m_fElapsedTime;

	if (input[0].age <= gEffect.m_fDuration)
	{
		input[0].size.x *= 1.0f - input[0].age / gEffect.m_fDuration;
		outStream.Append(input[0]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct FOLLOW_EFFECT
{
	float3	m_f3Position;
	float	m_fElapsedTime;
	float	m_fDuration;
};

ConstantBuffer<FOLLOW_EFFECT> gFollowEffect : register(FOLLOW_EFFECT_INFO);

[maxvertexcount(4)]
void GSFollowEffectDraw(point VS_EFFECT_OUTPUT input[1], inout TriangleStream<GS_EFFECT_OUTPUT> outStream)
{
	float3 pos = gFollowEffect.m_f3Position + input[0].position;

	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - pos);
	float3 vRight = normalize(cross(vUp, vLook));
	vUp = normalize(cross(vRight, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(pos + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(pos + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(pos - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(pos - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_EFFECT_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSFollowEffectDraw(GS_EFFECT_OUTPUT input) : SV_TARGET1
{
	return gtxtTexture[0].Sample(gssWrap, input.uv) * input.color;
}

[maxvertexcount(1)]
void GSFollowEffectStreamOut(point VS_EFFECT_INPUT input[1], inout PointStream<VS_EFFECT_INPUT> outStream)
{
	input[0].age += gEffect.m_fElapsedTime;

	outStream.Append(input[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

[maxvertexcount(4)]
void GSTextEffectDraw(point VS_EFFECT_OUTPUT input[1], inout TriangleStream<GS_EFFECT_OUTPUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3 vRight = normalize(cross(vUp, vLook));
	float fDistance = distance(gvCameraPosition.xyz, input[0].position);

	float fHalfW = input[0].size.x + fDistance * input[0].size.x;
	float fHalfH = input[0].size.y + fDistance * input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_EFFECT_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define EFFECT_TYPE_SPRITE_ONE 0
#define EFFECT_TYPE_SPRITE_LOOP 1

struct SPRITE
{
	float2 m_f2SpriteSize;
	uint m_nMaxSpriteX;
	uint m_nMaxSpriteY;
	uint m_nMaxSprite;
	float m_fDurationPerSprite;
};

ConstantBuffer<SPRITE> gSprite : register(SPRITE_INFO);

struct VS_SPRITE_INPUT
{
	float3	position : POSITION;
	float2	size : SIZE;
	uint2	spritepos : SPRITEPOS;
	float	age : AGE;
	uint	type : TYPE;
	int		angle : ANGLE;
	float4	color : COLOR;
};

struct VS_SPRITE_OUTPUT
{
	float3	position : POSITION;
	float2	size : SIZE;
	uint2	spritepos : SPRITEPOS;
	int		angle : ANGLE;
	float4	color : COLOR;
};

struct GS_SPRITE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};

VS_SPRITE_OUTPUT VSSpriteDraw(VS_SPRITE_INPUT input)
{
	VS_SPRITE_OUTPUT output;

	output.position = input.position;
	output.size = input.size;
	output.spritepos = input.spritepos;
	output.angle = input.angle;
	output.color = input.color;

	return output;
}

[maxvertexcount(4)]
void GSSpriteDraw(point VS_SPRITE_OUTPUT input[1], inout TriangleStream<GS_SPRITE_OUTPUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3x3 f3x3Rotate = RotateAxis(vLook, input[0].angle);
	vUp = mul(vUp, f3x3Rotate);
	float3 vRight = normalize(cross(vUp, vLook));
	vUp = normalize(cross(vRight, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_SPRITE_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		float3x3 f3x3Sprite = float3x3(gSprite.m_f2SpriteSize.x, 0.0f, 0.0f, 0.0f, gSprite.m_f2SpriteSize.y, 0.0f, fUVs[i].x * gSprite.m_f2SpriteSize.x, fUVs[i].y * gSprite.m_f2SpriteSize.y, 1.0f);
		float3 f3Sprite = float3(input[0].spritepos, 1.0f);
		output.uv = (float2)mul(f3Sprite, f3x3Sprite);

		outStream.Append(output);
	}
}

PS_EFFECT_OUTPUT PSSpriteDraw(GS_SPRITE_OUTPUT input) : SV_TARGET
{
	PS_EFFECT_OUTPUT output;

	output.color = output.glow = gtxtTexture[0].Sample(gssWrap, input.uv)* input.color;

	return output;
}

VS_SPRITE_INPUT VSSpriteStreamOut(VS_SPRITE_INPUT input)
{
	return input;
}

[maxvertexcount(1)]
void GSSpriteStreamOut(point VS_SPRITE_INPUT input[1], inout PointStream<VS_SPRITE_INPUT> outStream)
{
	input[0].age += gEffect.m_fElapsedTime;

	if (input[0].age > gEffect.m_fDuration)
	{
		if(input[0].type == EFFECT_TYPE_SPRITE_ONE)  return;
		else if (input[0].type == EFFECT_TYPE_SPRITE_LOOP) input[0].age = fmod(input[0].age, gEffect.m_fDuration);
	}

	uint nSpritePos = int(input[0].age / gSprite.m_fDurationPerSprite);

	input[0].spritepos.x = nSpritePos % gSprite.m_nMaxSpriteX; 
	input[0].spritepos.y = int(nSpritePos / gSprite.m_nMaxSpriteX);

	outStream.Append(input[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

[maxvertexcount(4)]
void GSFollowSpriteDraw(point VS_SPRITE_OUTPUT input[1], inout TriangleStream<GS_SPRITE_OUTPUT> outStream)
{
	float3 pos = gFollowEffect.m_f3Position + input[0].position;

	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - pos);
	float3 vRight = normalize(cross(vUp, vLook));
	vUp = normalize(cross(vRight, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(pos + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(pos + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(pos - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(pos - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_SPRITE_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(fVertices[i], gmtxViewProjection);
		float3x3 f3x3Sprite = float3x3(gSprite.m_f2SpriteSize.x, 0.0f, 0.0f, 0.0f, gSprite.m_f2SpriteSize.y, 0.0f, fUVs[i].x * gSprite.m_f2SpriteSize.x, fUVs[i].y * gSprite.m_f2SpriteSize.y, 1.0f);
		float3 f3Sprite = float3(input[0].spritepos, 1.0f);
		output.uv = (float2)mul(f3Sprite, f3x3Sprite);

		outStream.Append(output);
	}
}

PS_EFFECT_OUTPUT PSFollowSpriteDraw(GS_SPRITE_OUTPUT input) : SV_TARGET
{
	PS_EFFECT_OUTPUT output;

	output.color = gtxtTexture[0].Sample(gssWrap, input.uv) * input.color;
	output.glow = gtxtTexture[0].Sample(gssWrap, input.uv) * input.color;

	return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PARTICLE_TYPE_COMMON 0
#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_ONE_EMITTER 2

struct PARTICLE
{
	float3	m_vPosition;
	float	m_fSpeed;
	float3	m_vDirection;
	float	m_fDuration;
	float3  m_vRight;
	float	m_fElapsedTime;
	float3  m_vUp;
	float	m_fMass;
	float3  m_vLook;
	bool	m_bEmit;
	float3	m_vAngles;
	bool	m_bScaling;
};

ConstantBuffer<PARTICLE> gParticle : register(PARTICLE_INFO);

struct VS_PARTICLE_INPUT
{
	float3	position : POSITION;
	float3	velocity : VELOCITY;
	float2	size : SIZE;
	int		type : TYPE;
	float	age : AGE;
	int		angle : ANGLE;
	float4	color : COLOR;
};

struct VS_PARTICLE_SO_OUTPUT
{
	float3	position : POSITION;
	float3	velocity : VELOCITY;
	float2	size : SIZE;
	int		type : TYPE;
	float	age : AGE;
	float	verid : VERTEXID;
	int		angle : ANGLE;
	float4	color : COLOR;
};

VS_PARTICLE_SO_OUTPUT VSParticleStreamOut(VS_PARTICLE_INPUT input, uint nVerID : SV_VertexID)
{
	VS_PARTICLE_SO_OUTPUT output;
	output.position = input.position;
	output.velocity = input.velocity;
	output.size = input.size;
	output.type = input.type;
	output.age = input.age;
	output.verid = float(nVerID);
	output.angle = input.angle;
	output.color = input.color;

	return output;
}

[maxvertexcount(2)]
void GSParticleStreamOut(point VS_PARTICLE_SO_OUTPUT input[1], inout PointStream<VS_PARTICLE_INPUT> pointStream)
{
	VS_PARTICLE_INPUT output;
	output.position = input[0].position;
	output.velocity = input[0].velocity;
	output.size = input[0].size;
	output.type = input[0].type;
	output.age = input[0].age + gParticle.m_fElapsedTime;
	output.angle = input[0].angle;
	output.color = input[0].color;

	if ((input[0].type == PARTICLE_TYPE_EMITTER) || (input[0].type == PARTICLE_TYPE_ONE_EMITTER))
	{
		if (gParticle.m_bEmit == true)
		{
			float4 vRandom = gvRandoms * (input[0].verid * input[0].verid + 5.0f);
			float fX = fmod(vRandom.x, gParticle.m_vAngles.x) - gParticle.m_vAngles.x / 2.0f;
			float fY = fmod(vRandom.y, gParticle.m_vAngles.y) - gParticle.m_vAngles.y / 2.0f;
			float fZ = fmod(vRandom.z, gParticle.m_vAngles.z) - gParticle.m_vAngles.z / 2.0f;

			float3 vDirection = gParticle.m_vDirection;

			if (fX > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vRight, fX));
			if (fY > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vUp, fY));
			if (fZ > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vLook, fZ));

			float f = (frac(vRandom.x) + 1.0f);
			VS_PARTICLE_INPUT particle;
			particle.position = output.position + gParticle.m_vPosition;
			particle.velocity = gParticle.m_fSpeed * f * vDirection;
			particle.size = output.size * f;
			particle.type = PARTICLE_TYPE_COMMON;
			particle.age = 0.0f;
			particle.angle = vRandom.w / 360.0f;
			particle.color = output.color;

			pointStream.Append(particle);

			output.age = 0.0f;
		}

		if (input[0].type == PARTICLE_TYPE_EMITTER)
			pointStream.Append(output);
	}
	else
	{
		if (output.age < gParticle.m_fDuration) pointStream.Append(output);
	}
}

////////////////////////////////////////////

struct VS_PARTICLE_OUTPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	float4 color : COLOR;
	uint type : TYPE;
	int angle : ANGLE;
};

struct GS_PARTICLE_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

VS_PARTICLE_OUTPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
	VS_PARTICLE_OUTPUT output;

	float t = input.age;
	float3 vGravity = float3(0, gfGravity, 0);
	output.position = (input.velocity * t) + (vGravity * t * t * gParticle.m_fMass) + input.position;

	float fOpacity = 1.0f - smoothstep(0.0f, gParticle.m_fDuration, input.age);
	output.color = float4(input.color.rgb, fOpacity);

	output.size = input.size;

	if (gParticle.m_bScaling)
		output.size *= input.age;

	output.type = input.type;
	output.angle = input.angle;

	return output;
}

static float2 gvQuadTexCoord[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;

	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3x3 f3x3Rotate = RotateAxis(vLook, input[0].angle);
	vUp = mul(vUp, f3x3Rotate);
	float3 vRight = normalize(cross(vUp, vLook));
	vUp = normalize(cross(vRight, vLook));

	float fHalfWidth = 0.5f * input[0].size.x;
	float fHalfHeight = 0.5f * input[0].size.y;

	float4 vQuads[4];
	vQuads[0] = float4(input[0].position + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuads[1] = float4(input[0].position + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);
	vQuads[2] = float4(input[0].position - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuads[3] = float4(input[0].position - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);

	GS_PARTICLE_OUTPUT output;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(vQuads[i], gmtxViewProjection);
		output.uv = gvQuadTexCoord[i];
		triStream.Append(output);
	}
}

PS_EFFECT_OUTPUT PSParticleDraw(GS_PARTICLE_OUTPUT input) : SV_TARGET
{
	PS_EFFECT_OUTPUT output;
	output.color = output.glow = gtxtTexture[0].Sample(gssWrap, input.uv)* input.color;

	return output;
}
