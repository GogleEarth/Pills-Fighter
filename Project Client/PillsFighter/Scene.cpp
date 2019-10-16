#include "stdafx.h"
#include "Scene.h"
#include "Repository.h"
#include "Weapon.h"
#include "Effect.h"

ID3D12DescriptorHeap			*CScene::m_pd3dSrvUavDescriptorHeap = NULL;
ID3D12RootSignature				*CScene::m_pd3dGraphicsRootSignature = NULL;
ID3D12RootSignature				*CScene::m_pd3dComputeRootSignature = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvUavTextureCPUDescStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvUavTextureGPUDescStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvModelCPUDescStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvModelGPUDescStartHandle;

int								CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

int								CScene::m_nMyTeam = TEAM_TYPE::TEAM_TYPE_RED;
wchar_t							CScene::m_pwstrMyName[10] = L"";
bool							CScene::m_bInitName = false;
int								CScene::m_nCurrentScene = SCENE_TYPE_TITLE;

extern CFMODSound gFmodSound;

CScene::CScene()
{
	CONSOLE_CURSOR_INFO C;
	C.bVisible = 0;
	C.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &C);
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	m_pFontShader = new CFontShader();
	m_pFontShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pFontShader)
	{
		m_pFontShader->ReleaseShaderVariables();
		m_pFontShader->ReleaseObjects();
		delete m_pFontShader;

		m_pFontShader = NULL;
	}

	if (m_pFont)
		m_pFont->ClearTexts();
}

void CScene::SetFont(ID3D12Device *pd3dDevice, CFont *pFont)
{
	pFont->SetSrv(pd3dDevice);
	m_pFont = pFont;
}

int CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CScene::ProcessInput(float fTimeElapsed)
{
}

void CScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	CheckCollision();
}

void CScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pFontShader) m_pFontShader->OnPrepareRender(pd3dCommandList, 0);
	if (m_pFont) m_pFont->Render(pd3dCommandList);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType, DXGI_FORMAT dxFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = dxFormat;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

// statics

void CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 3;
	pd3dDescriptorRanges[0].BaseShaderRegister = 1; //t1 ~ t3: gtxtTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 4; //t4: Specular
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 5; //t5: Normal
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 6; //t6: Skybox
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 5;
	pd3dDescriptorRanges[4].BaseShaderRegister = 7; // t7~10: tile*4 + Setting
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 13; // t13: Envir
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 14; //t14: Emissive
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 15; //t15: Shadow map
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 16; //t16: Normal map
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[MAX_ROOT_PARAMETER_INDEX];

	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].Descriptor.ShaderRegister = 0; //Object
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].Descriptor.ShaderRegister = 2; //Lights
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; // t1: Diffuse Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; // t4: Specular Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; // t5: Normal Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6]; // t14: Emissive Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; // t6: Texture Cube
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.ShaderRegister = 3; //HP
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].Descriptor.ShaderRegister = 5; //Offset
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].Descriptor.ShaderRegister = 6; //Transforms
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4]; // t7~10
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].Descriptor.ShaderRegister = 7; // Effect
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].Descriptor.ShaderRegister = 4; //TextureSprite
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].Descriptor.ShaderRegister = 8;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].Descriptor.ShaderRegister = 12;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5]; // t6: Texture Cube
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].Descriptor.ShaderRegister = 12;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[7];
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].Descriptor.ShaderRegister = 13;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_RELOAD_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_RELOAD_INFO].Descriptor.ShaderRegister = 15;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_RELOAD_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_RELOAD_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ENEMY_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ENEMY_INFO].Descriptor.ShaderRegister = 16;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ENEMY_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ENEMY_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].Descriptor.ShaderRegister = 17;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_MAP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_MAP].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_MAP].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[8];
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_MAP].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_FOLLOW_EFFECT_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FOLLOW_EFFECT_INFO].Descriptor.ShaderRegister = 20;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FOLLOW_EFFECT_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FOLLOW_EFFECT_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_3D_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_3D_INFO].Descriptor.ShaderRegister = 19;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_3D_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_3D_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].Descriptor.ShaderRegister = 11;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

/////
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.Num32BitValues = 2;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.ShaderRegister = 10;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCREEN_EFFECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCREEN_EFFECT].Constants.Num32BitValues = 4;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCREEN_EFFECT].Constants.ShaderRegister = 14;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCREEN_EFFECT].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCREEN_EFFECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_COLOR_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_COLOR_INFO].Constants.Num32BitValues = 4;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_COLOR_INFO].Constants.ShaderRegister = 18;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_COLOR_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_COLOR_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_TEAM_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_TEAM_INFO].Descriptor.ShaderRegister = 21;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_TEAM_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_TEAM_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_CUSTOM_UI_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CUSTOM_UI_INFO].Descriptor.ShaderRegister = 22;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CUSTOM_UI_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CUSTOM_UI_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void CScene::CreateComputeRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 3;
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 0;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 4;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[MAX_COMPUTE_ROOT_PARAMETER_INDEX];

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]); // UAV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.Num32BitValues = 35;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.ShaderRegister = 0;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&m_pd3dComputeRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void CScene::CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dSrvUavDescriptorHeap);

	m_d3dSrvModelCPUDescStartHandle = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvModelGPUDescStartHandle = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	m_d3dSrvUavTextureCPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
	m_d3dSrvUavTextureGPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement, bool bIsModelTexture)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUDescHandle;

	if (bIsModelTexture) d3dGPUDescHandle = m_d3dSrvModelGPUDescStartHandle;
	else d3dGPUDescHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();

		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType, d3dResourceDesc.Format);

			D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSrvCPUDescHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE *pd3dSrvGPUDescHandle;

			if (bIsModelTexture)
			{
				pd3dSrvCPUDescHandle = &m_d3dSrvModelCPUDescStartHandle;
				pd3dSrvGPUDescHandle = &m_d3dSrvModelGPUDescStartHandle;
			}
			else
			{
				pd3dSrvCPUDescHandle = &m_d3dSrvUavTextureCPUDescStartHandle;
				pd3dSrvGPUDescHandle = &m_d3dSrvUavTextureGPUDescStartHandle;
			}

			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, *pd3dSrvCPUDescHandle);

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, *pd3dSrvGPUDescHandle);

			(*pd3dSrvCPUDescHandle).ptr += ::gnCbvSrvDescriptorIncrementSize;
			(*pd3dSrvGPUDescHandle).ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}

	return d3dGPUDescHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType, bool bIsDS)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pd3dResource)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;

		if (bIsDS)
			d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nSrvType, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		else
			d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nSrvType, d3dResourceDesc.Format);

		pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_d3dSrvUavTextureCPUDescStartHandle);

		m_d3dSrvUavTextureCPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_d3dSrvUavTextureGPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

	return d3dSrvGPUDescriptorHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGPUDescriptorHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pd3dResource)
	{
		pd3dDevice->CreateUnorderedAccessView(pd3dResource, NULL, NULL, m_d3dSrvUavTextureCPUDescStartHandle);

		m_d3dSrvUavTextureCPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_d3dSrvUavTextureGPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

	return d3dUavGPUDescriptorHandle;
}

void CScene::ReleaseDescHeapsAndRootSignature()
{
	if (m_pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature->Release();
		m_pd3dGraphicsRootSignature = NULL;
	}

	if (m_pd3dComputeRootSignature)
	{
		m_pd3dComputeRootSignature->Release();
		m_pd3dComputeRootSignature = NULL;
	}

	if (m_pd3dSrvUavDescriptorHeap)
	{
		m_pd3dSrvUavDescriptorHeap->Release();
		m_pd3dSrvUavDescriptorHeap = NULL;
	}
}

void CScene::SetDescHeapsAndGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature)	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dSrvUavDescriptorHeap)		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvUavDescriptorHeap);
}

void CScene::ResetDescriptorHeapHandles()
{
	m_d3dSrvUavTextureCPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
	m_d3dSrvUavTextureGPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CUIScene::CUIScene()
{

}

CUIScene::~CUIScene()
{

}

int CUIScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONUP:
		return MouseClick();
		break;
	default:
		break;
	}

	return 0;
}

void CUIScene::MoveCursor(float x, float y)
{
	if (m_pCursor) m_pCursor->MoveCursorPos(x, y);
}

void CUIScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	if (!m_pLobbyShader)
	{
		m_pLobbyShader = new CLobbyShader();
		m_pLobbyShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
		m_pLobbyShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	}

	if (!m_pCursor) m_pCursor = new CCursor(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

	m_d3dViewport = { 0.0f, 0.0f, float(FRAME_BUFFER_WIDTH), float(FRAME_BUFFER_HEIGHT), 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
}

void CUIScene::ReleaseObjects()
{
	CScene::ReleaseObjects();

	if (m_ppUIRects)
	{
		for (int i = 0; i < m_nUIRect; i++)
		{
			if (m_ppUIRects[i])
			{
				m_ppUIRects[i]->ReleaseShaderVariables();
				delete m_ppUIRects[i];
			}
		}
		delete[] m_ppUIRects;
		m_ppUIRects = NULL;
	}

	if (m_ppTextures)
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_ppTextures[i])
				delete m_ppTextures[i];
		}
		delete[] m_ppTextures;
		m_ppTextures = NULL;
	}

	if (m_pLobbyShader)
	{
		m_pLobbyShader->ReleaseObjects();
		m_pLobbyShader->ReleaseShaderVariables();
		delete m_pLobbyShader;
		m_pLobbyShader = NULL;
	}

	if (m_pCursor)
	{
		delete m_pCursor;
		m_pCursor = NULL;
	}
}

void CUIScene::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();

	if (m_ppUIRects)
	{
		for (int i = 0; i < m_nUIRect; i++)
		{
			if (m_ppUIRects[i]) m_ppUIRects[i]->ReleaseUploadBuffers();
		}
	}

	if (m_ppTextures)
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pCursor) m_pCursor->ReleaseUploadBuffer();
}

void CUIScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	CScene::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTitleScene::CTitleScene()
{

}

CTitleScene::~CTitleScene()
{

}

void CTitleScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CUIScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	m_nTextures = TITLE_UI_TEXTURE_COUNT;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[TITLE_UI_TEXTURE_BASE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_BASE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/Title.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_BASE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[TITLE_UI_TEXTURE_START] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_START]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/Start.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_START], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[TITLE_UI_TEXTURE_HL_START] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_HL_START]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/StartHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_HL_START], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[TITLE_UI_TEXTURE_EXIT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_EXIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/Exit.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_EXIT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[TITLE_UI_TEXTURE_HL_EXIT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_HL_EXIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/ExitHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_HL_EXIT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[TITLE_UI_TEXTURE_NOTIFY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TITLE_UI_TEXTURE_NOTIFY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/NotifyFail.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TITLE_UI_TEXTURE_NOTIFY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	// Base UI

	m_nUIRect = TITLE_UI_RECT_COUNT;
	m_ppUIRects = new CRect*[m_nUIRect];

	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[TITLE_UI_RECT_BASE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	m_ppUIRects[TITLE_UI_RECT_SCREEN] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.0f;
	float centery = -0.27f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[TITLE_UI_RECT_START] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	centery = -0.61f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[TITLE_UI_RECT_EXIT] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 400.0f / FRAME_BUFFER_WIDTH;
	height = 200.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.0f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[TITLE_UI_RECT_NOTIFY] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
}

void CTitleScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CUIScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.0f;
	float centery = -0.27f;
	m_StartButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_StartButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.0f;
	centery = -0.61f;
	m_ExitButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ExitButton.Extents = XMFLOAT3(width, height, 1.0f);
}

void CTitleScene::Notify()
{
	m_bShowNotify = true;
	m_fShowTime = 5.0f;
}

void CTitleScene::HideNotify()
{
	m_bShowNotify = false;
	m_fShowTime = 0.0f;
}

int CTitleScene::MouseClick()
{
	if (!m_bShowNotify)
	{
		if (m_pCursor->CollisionCheck(m_StartButton))
		{
#ifdef ON_NETWORKING
			WSADATA wsa;
			if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			{
				Notify();
			}

			gSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (gSocket == INVALID_SOCKET)
			{
				Notify();
				return 0;
			}

			char strIP[32];
			::ZeroMemory(strIP, sizeof(strIP));

			FILE *file;
			fopen_s(&file, "Server.txt", "r");
			fscanf(file, "%s", strIP);

			SOCKADDR_IN serveraddr;
			ZeroMemory(&serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_addr.s_addr = inet_addr(strIP);
			serveraddr.sin_port = htons(SERVERPORT);

			if (connect(gSocket, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
			{
				Notify();
				return 0;
			}
#endif

			return LOBBY_MOUSE_CLICK_GAME_START;
		}
		if (m_pCursor->CollisionCheck(m_ExitButton))
		{
			return LOBBY_MOUSE_CLICK_GAME_EXIT;
		}
	}
	else
	{
		HideNotify();
	}

	return 0;
}

void CTitleScene::CheckCollision()
{
	if (!m_bShowNotify)
	{
		if (m_pCursor->CollisionCheck(m_StartButton)) m_bHLStartButton = true;
		else m_bHLStartButton = false;

		if (m_pCursor->CollisionCheck(m_ExitButton)) m_bHLExitButton = true;
		else m_bHLExitButton = false;
	}
}

void CTitleScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_bShowNotify)
	{
		m_fShowTime -= fTimeElapsed;

		if (m_fShowTime <= 0.0f)
		{
			HideNotify();
		}
	}

	CheckCollision();
}

void CTitleScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	m_ppTextures[TITLE_UI_TEXTURE_BASE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[TITLE_UI_RECT_BASE]->Render(pd3dCommandList, 0);

	if (m_bHLStartButton)
		m_ppTextures[TITLE_UI_TEXTURE_HL_START]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[TITLE_UI_TEXTURE_START]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[TITLE_UI_RECT_START]->Render(pd3dCommandList, 0);

	if (m_bHLExitButton)
		m_ppTextures[TITLE_UI_TEXTURE_HL_EXIT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[TITLE_UI_TEXTURE_EXIT]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[TITLE_UI_RECT_EXIT]->Render(pd3dCommandList, 0);

	if (m_bShowNotify)
	{
		if (m_pLobbyShader) m_pLobbyShader->SetScreenPipelineState(pd3dCommandList);
		m_ppUIRects[TITLE_UI_RECT_SCREEN]->Render(pd3dCommandList, 0);

		if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);
		m_ppTextures[TITLE_UI_TEXTURE_NOTIFY]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[TITLE_UI_RECT_NOTIFY]->Render(pd3dCommandList, 0);
	}

	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

void CTitleScene::StartScene(bool bBGMStop)
{
	if (bBGMStop) gFmodSound.StopFMODSound(gFmodSound.m_pBGMChannel);
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundTitleBGM, &(gFmodSound.m_pBGMChannel));
}

void CTitleScene::EndScene()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyMainScene::CLobbyMainScene()
{

}

CLobbyMainScene::~CLobbyMainScene()
{

}

int CLobbyMainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_bActNameChange || m_bActInputRoomName)
	{
		wchar_t *before = m_InputTextObject->GetText();

		m_pTextSystem->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

		wchar_t *after = m_pTextSystem->GetText();

		if (lstrcmpW(before, after))
		{
			if(m_bActNameChange) ChangeInputNameText();
			else if(m_bActInputRoomName) ChangeInputRoomNameText();
		}

		switch (nMessageID)
		{
		case WM_KEYDOWN:

			switch (wParam)
			{
			case VK_RETURN:
				if (m_bActNameChange)
				{
					m_bActNameChange = false;
					lstrcpynW(CScene::m_pwstrMyName, m_pTextSystem->GetText(), sizeof(CScene::m_pwstrMyName));
					return LOBBY_KEYDOWN_CHANGE_NAME;
				}
				else if(m_bActInputRoomName)
				{
					m_bActInputRoomName = false;
					lstrcpynW(m_pwstrRoomName, m_pTextSystem->GetText(), sizeof(CScene::m_pwstrMyName));
					return LOBBY_KEYDOWN_CREATE_ROOM;
				}
				break;
			case VK_ESCAPE:
				if (m_bActNameChange) m_bActNameChange = false;
				else if (m_bActInputRoomName) m_bActInputRoomName = false;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	return 0;
}

void CLobbyMainScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CUIScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	m_nTextures = LOBBY_MAIN_UI_TEXTURE_COUNT;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/MainLobby.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/CreateRoom.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/CreateRoomHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CHANGE_NAME] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CHANGE_NAME]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/ChangeName.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CHANGE_NAME], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CHANGE_NAME] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CHANGE_NAME]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/ChangeNameHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CHANGE_NAME], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/Up.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/UpHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/Down.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/DownHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/RoomHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_INPUT_BOX] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_INPUT_BOX]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/InputBox.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_INPUT_BOX], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_EXIT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_EXIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/Exit.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_EXIT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_EXIT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_EXIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Title/ExitHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_EXIT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_NOTIFY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_NOTIFY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/Notify.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_NOTIFY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	// Base UI
	m_nUIRect = LOBBY_MAIN_UI_RECT_COUNT;
	m_ppUIRects = new CRect*[m_nUIRect];

	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_BASE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_SCREEN] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.796375f;
	float centery = -0.371222f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CREATE_ROOM_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	centery -= 0.23453f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CHANGE_NAME_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	centery -= 0.22853f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_EXIT_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 22.0f / FRAME_BUFFER_WIDTH;
	height = 12.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.8f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_UP_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	centery = -0.173f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_DOWN_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 440.0f / FRAME_BUFFER_WIDTH;
	height = 100.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.0f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_INPUT_BOX] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 850.0f / FRAME_BUFFER_WIDTH;
	height = 100.0f / FRAME_BUFFER_HEIGHT;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_INPUT_ROOM_NAME] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 400.0f / FRAME_BUFFER_WIDTH;
	height = 200.0f / FRAME_BUFFER_HEIGHT;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_NOTIFY] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	for (int i = 0; i < 8; i++)
	{
		width = 5.0f / FRAME_BUFFER_WIDTH;
		height = 38.0f / FRAME_BUFFER_HEIGHT;
		centerx = -0.952f;
		centery = 0.7225f - 0.1162f * i;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_ROOM_BUTTON_1 + i] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	}

	::memset(m_bHLRoomButton, false, sizeof(m_bHLRoomButton));
	::ZeroMemory(m_pwstrRoomName, sizeof(m_pwstrRoomName));
}

void CLobbyMainScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CUIScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.796375f;
	float centery = -0.371222f;
	m_CreateRoomButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_CreateRoomButton.Extents = XMFLOAT3(width, height, 1.0f);

	centery -= 0.23453f;
	m_NameChangeButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_NameChangeButton.Extents = XMFLOAT3(width, height, 1.0f);

	centery -= 0.22853f;
	m_ExitButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ExitButton.Extents = XMFLOAT3(width, height, 1.0f);

	width = 1210.0f / FRAME_BUFFER_WIDTH;
	height = 12.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.8f;
	m_UpButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_UpButton.Extents = XMFLOAT3(width, height, 1.0f);

	centery = -0.173f;
	m_DownButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_DownButton.Extents = XMFLOAT3(width, height, 1.0f);

	height = 38.0f / FRAME_BUFFER_HEIGHT;
	for (int i = 0; i < 8; i++)
	{
		centery = 0.7225f - 0.1162f * i;

		m_RoomButton[i].Center = XMFLOAT3(centerx, centery, 1.0f);
		m_RoomButton[i].Extents = XMFLOAT3(width, height, 1.0f);
	}

#ifndef ON_NETWORKING
	AddRoom(1, L"ㄱㄱ");
	AddRoom(2, L"ㄱㄱ");
	AddRoom(3, L"ㄱㄱ");
	AddRoom(4, L"ㄱㄱ");
	AddRoom(5, L"ㄱㄱ");
	AddRoom(6, L"ㄱㄱ");
	AddRoom(7, L"ㄱㄱ");
	AddRoom(8, L"ㄱㄱ");
	AddRoom(9, L"ㄱㄱ");

	InitName(L"오프라인");
#endif
}

void CLobbyMainScene::ReleaseObjects()
{
	CUIScene::ReleaseObjects();
}

void CLobbyMainScene::ReleaseUploadBuffers()
{
	CUIScene::ReleaseUploadBuffers();
}

wchar_t* GetRandomRoomName()
{
	switch (rand() % 10)
	{
	case 0:
		return L"게임 같이 하실분";
	case 1:
		return L"4:4 초보만";
	case 2:
		return L"3:3 초보만";
	case 3:
		return L"2:2 초보만";
	case 4:
		return L"1:1 초보만";
	case 5:
		return L"아무나 들어오셈";
	case 6:
		return L"나보다 잘하는 사람만";
	case 7:
		return L"고수만";
	case 8:
		return L"ㅋㅋㅋ";
	case 9:
	default:
		return L"나를 눕혀봐";
	}
}

int CLobbyMainScene::MouseClick()
{
	if (!CScene::m_bInitName) return 0;

	if (!m_bActNameChange && !m_bActInputRoomName)
	{
		if (m_pCursor->CollisionCheck(m_CreateRoomButton))
		{
			lstrcpynW(m_pwstrRoomName, GetRandomRoomName(), sizeof(m_pwstrRoomName));
			m_pTextSystem->SetText(m_pwstrRoomName);
			m_pTextSystem->SetTextLength(MAX_ROOM_NAME_LENGTH);
			ChangeInputRoomNameText();
			m_bActInputRoomName = true;
		}

		if (m_pCursor->CollisionCheck(m_NameChangeButton))
		{
			m_pTextSystem->SetText(CScene::m_pwstrMyName);
			m_pTextSystem->SetTextLength(MAX_NAME_LENGTH);
			ChangeInputNameText();
			m_bActNameChange = true;
		}

		if (m_pCursor->CollisionCheck(m_ExitButton))
		{
			::PostQuitMessage(0);
		}

		if (m_RoomStart > 0)
		{
			if (m_pCursor->CollisionCheck(m_UpButton))
			{
				m_RoomStart--;

				for (auto& Room : m_Rooms)
				{
					Room.pRoom_map->MovePosition(XMFLOAT3(0.0f, -0.1162f, 0.0f));
					Room.pRoom_name->MovePosition(XMFLOAT3(0.0f, -0.1162f, 0.0f));
					Room.pRoom_num->MovePosition(XMFLOAT3(0.0f, -0.1162f, 0.0f));
					Room.pRoom_num_people->MovePosition(XMFLOAT3(0.0f, -0.1162f, 0.0f));
				}
			}
		}

		if (m_Rooms.size() - m_RoomStart > 8)
		{
			if (m_pCursor->CollisionCheck(m_DownButton))
			{
				m_RoomStart++;

				for (auto& Room : m_Rooms)
				{
					Room.pRoom_map->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
					Room.pRoom_name->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
					Room.pRoom_num->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
					Room.pRoom_num_people->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
				}
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (m_Rooms.size() - m_RoomStart <= i) continue;

			if (m_pCursor->CollisionCheck(m_RoomButton[i]))
			{
				m_nSelectRoom = m_RoomStart + i;
#ifndef ON_NETWORKING
				//DeleteRoom(m_Rooms[m_nSelectRoom].nRoom_num);
				ChangeRoomInfo(m_Rooms[m_nSelectRoom].nRoom_num, SCENE_TYPE_SPACE, 5);
#endif
				return LOBBY_MOUSE_CLICK_JOIN_ROOM;
			}
		}
	}
	else
	{
		if (m_bActNameChange) m_bActNameChange = false;
		else if (m_bActInputRoomName) m_bActInputRoomName = false;
	}

	return 0;
}

void CLobbyMainScene::CheckCollision()
{
	if (!CScene::m_bInitName) return;
	if (m_bActNameChange) return;
	if (m_bActInputRoomName) return;

	if (m_pCursor->CollisionCheck(m_CreateRoomButton)) m_bHLCreateRoomButton = true;
	else m_bHLCreateRoomButton = false;

	if (m_pCursor->CollisionCheck(m_UpButton)) m_bHLUpButton = true;
	else m_bHLUpButton = false;

	if (m_pCursor->CollisionCheck(m_DownButton)) m_bHLDownButton = true;
	else m_bHLDownButton = false;

	for (int i = 0; i < 8; i++)
	{
		m_bHLRoomButton[i] = false;
		if (m_Rooms.size() - m_RoomStart <= i) continue;

		if (m_pCursor->CollisionCheck(m_RoomButton[i])) m_bHLRoomButton[i] = true;
	}

	if (m_pCursor->CollisionCheck(m_NameChangeButton)) m_bHLNameChangeButton = true;
	else m_bHLNameChangeButton = false;

	if (m_pCursor->CollisionCheck(m_ExitButton)) m_bHLExitButton = true;
	else m_bHLExitButton = false;
}

void CLobbyMainScene::ChangeInputNameText()
{
	m_pFont->ChangeText(m_InputTextObject, m_pTextSystem->GetText(), XMFLOAT2(3.0f, 3.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);
	m_InputTextObject->SetPosition(XMFLOAT2(-0.325f, 0.06f));
}

void CLobbyMainScene::ChangeInputRoomNameText()
{
	m_pFont->ChangeText(m_InputTextObject, m_pTextSystem->GetText(), XMFLOAT2(3.0f, 3.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);
	m_InputTextObject->SetPosition(XMFLOAT2(-0.625f, 0.06f));
}

void CLobbyMainScene::InitName(wchar_t *pwstrName)
{
	lstrcpynW(CScene::m_pwstrMyName, pwstrName, MAX_NAME_LENGTH);
	m_InputTextObject = m_pFont->SetText(CScene::m_pwstrMyName, XMFLOAT2(-0.325f, 0.06f), XMFLOAT2(3.0f, 3.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	m_pTextSystem->SetText(CScene::m_pwstrMyName);

	CScene::m_bInitName = true;
}

void CLobbyMainScene::AddRoom(int n, wchar_t *name)
{
	ROOM_INFO_TEXT newRoom;

	wchar_t pstrNumber[6];
	wsprintfW(pstrNumber, L"%d", n);
	newRoom.nRoom_num = n;

	float yPos = (m_RoomStart + m_Rooms.size())* 0.1162f;
	XMFLOAT2 xmf2Position;

	xmf2Position = XMFLOAT2(-0.943750f, 0.765222f - yPos);
	newRoom.pRoom_num = m_pFont->SetText(pstrNumber, xmf2Position, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	xmf2Position.x = -0.806250f;
	newRoom.pRoom_name = m_pFont->SetText(name, xmf2Position, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	xmf2Position.x = 0.389063f;
	newRoom.pRoom_map = m_pFont->SetText(L"콜로니", xmf2Position, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	xmf2Position.x = 0.776562f;
	newRoom.pRoom_num_people = m_pFont->SetText(L"1/8", xmf2Position, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);


	m_Rooms.emplace_back(newRoom);
}

void CLobbyMainScene::DeleteRoom(int n)
{
	int nStart = -1;

	for (int i = 0; i < m_Rooms.size(); i++)
	{
		if (m_Rooms[i].nRoom_num == n)
		{
			nStart = i;
			break;
		}
	}

	for (auto& Room = m_Rooms.begin(); Room != m_Rooms.end(); Room++)
	{
		if (Room->nRoom_num == n)
		{
			m_Rooms.erase(Room);
			break;
		}
	}

	for (int i = nStart; i < m_Rooms.size(); i++)
	{
		m_Rooms[i].pRoom_num->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
		m_Rooms[i].pRoom_name->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
		m_Rooms[i].pRoom_map->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
		m_Rooms[i].pRoom_num_people->MovePosition(XMFLOAT3(0.0f, 0.1162f, 0.0f));
	}
}

void CLobbyMainScene::ChangeRoomInfo(int index, int map, int people)
{
	wchar_t pstr[6];
	wsprintfW(pstr, L"%d/8", people);

	for (auto& room : m_Rooms)
	{
		if (room.nRoom_num == index)
		{
			if (map == SCENE_TYPE_COLONY)
				m_pFont->ChangeText(room.pRoom_map, L"콜로니", XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);
			else if (map == SCENE_TYPE_SPACE)
				m_pFont->ChangeText(room.pRoom_map, L"스페이스", XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);

			m_pFont->ChangeText(room.pRoom_num_people, pstr, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);
		}
	}
}

void CLobbyMainScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_TEXTURE_BASE]->Render(pd3dCommandList, 0);

	if (m_bHLCreateRoomButton)
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CREATE_ROOM_BUTTON]->Render(pd3dCommandList, 0);

	if (m_bHLNameChangeButton)
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CHANGE_NAME]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CHANGE_NAME]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CHANGE_NAME_BUTTON]->Render(pd3dCommandList, 0);

	if (m_bHLExitButton)
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_EXIT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_EXIT]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_EXIT_BUTTON]->Render(pd3dCommandList, 0);

	if (m_RoomStart > 0)
	{
		if (m_bHLUpButton)
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_UP_BUTTON]->Render(pd3dCommandList, 0);
	}

	if (m_Rooms.size() - m_RoomStart > 8)
	{
		if (m_bHLDownButton)
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_DOWN_BUTTON]->Render(pd3dCommandList, 0);
	}

	for (int i = 0; i < 8; i++)
	{
		if (m_bHLRoomButton[i])
		{
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM]->UpdateShaderVariables(pd3dCommandList);
			m_ppUIRects[LOBBY_MAIN_UI_RECT_ROOM_BUTTON_1 + i]->Render(pd3dCommandList, 0);
		}
	}

	if (m_Rooms.size() > 0)
	{
		if (m_pFontShader) m_pFontShader->Render(pd3dCommandList, NULL);
		m_pFont->OnPrepareRender(pd3dCommandList);

		int nEnd = m_RoomStart + 8;
		if (nEnd > (int)m_Rooms.size()) nEnd = (int)m_Rooms.size();

		for (int i = m_RoomStart; i < nEnd; i++)
		{
			m_Rooms[i].pRoom_map->Render(pd3dCommandList);
			m_Rooms[i].pRoom_name->Render(pd3dCommandList);
			m_Rooms[i].pRoom_num->Render(pd3dCommandList);
			m_Rooms[i].pRoom_num_people->Render(pd3dCommandList);
		}
	}

	if (m_bActNameChange || m_bActInputRoomName)
	{
		if (m_pLobbyShader) m_pLobbyShader->SetScreenPipelineState(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_SCREEN]->Render(pd3dCommandList, 0);

		if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_INPUT_BOX]->UpdateShaderVariables(pd3dCommandList);

		if(m_bActNameChange) m_ppUIRects[LOBBY_MAIN_UI_RECT_INPUT_BOX]->Render(pd3dCommandList, 0);
		else if(m_bActInputRoomName) m_ppUIRects[LOBBY_MAIN_UI_RECT_INPUT_ROOM_NAME]->Render(pd3dCommandList, 0);

		if (m_pFontShader) m_pFontShader->Render(pd3dCommandList, NULL);
		m_pFont->OnPrepareRender(pd3dCommandList);
		m_InputTextObject->Render(pd3dCommandList);
	}

	if (!CScene::m_bInitName)
	{
		if (m_pLobbyShader) m_pLobbyShader->SetScreenPipelineState(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_SCREEN]->Render(pd3dCommandList, 0);

		if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_NOTIFY]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_NOTIFY]->Render(pd3dCommandList, 0);
	}

	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

void CLobbyMainScene::StartScene(bool bBGMStop)
{
	if(bBGMStop) gFmodSound.StopFMODSound(gFmodSound.m_pBGMChannel);
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundLobbyBGM, &(gFmodSound.m_pBGMChannel));
	gFmodSound.m_pBGMChannel->setVolume(0.3f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyRoomScene::CLobbyRoomScene()
{
	::ZeroMemory(m_pPlayerInfos, sizeof(m_pPlayerInfos));
}

CLobbyRoomScene::~CLobbyRoomScene()
{

}

void CLobbyRoomScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CUIScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	{
		m_nTextures = LOBBY_ROOM_UI_TEXTURE_COUNT;
		m_ppTextures = new CTexture*[m_nTextures];

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/RoomLobbyManager.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/RoomLobbyMember.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Start.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/StartHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Leave.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/LeaveHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Colony.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ColonyHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ColonySelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Space.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);


		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/SpaceHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/SpaceSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GM.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GMHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GMSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Gundam.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GundamHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GundamSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Zaku.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ZakuHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ZakuSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_COLONY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_COLONY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/MapColony.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_COLONY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_SPACE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_SPACE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/MapSpace.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_SPACE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/TeamRed.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/TeamRedHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/TeamBlue.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/TeamBlueHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);
	}

	{
		m_nUIRect = LOBBY_ROOM_UI_RECT_COUNT;
		m_ppUIRects = new CRect*[m_nUIRect];

		// Base UI

		XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
		XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_BASE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		float width = 185.0f / FRAME_BUFFER_WIDTH;
		float height = 50.0f / FRAME_BUFFER_HEIGHT;
		float centerx = 0.797187f;
		float centery = -0.375f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_START_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.797187f;
		centery = -0.8425f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_LEAVE_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);


		width = 90.0f / FRAME_BUFFER_WIDTH;
		height = 32.0f / FRAME_BUFFER_HEIGHT;
		centerx = 0.721875f;
		centery = 0.111111f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_COLONY] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.871875f;
		centery = 0.111111f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_SPACE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		width = 302.0f / FRAME_BUFFER_WIDTH;
		height = 274.0f / FRAME_BUFFER_HEIGHT;
		centerx = 0.792187f;
		centery = 0.541667f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_MapRect = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		width = 47.0f / FRAME_BUFFER_WIDTH;
		height = 22.0f / FRAME_BUFFER_HEIGHT;
		centerx = -0.562500f;
		centery = 0.866667f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_TEAM_RED] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.223438f;
		centery = 0.866667f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_TEAM_BLUE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		width = 58.0f / FRAME_BUFFER_WIDTH;
		height = 76.0f / FRAME_BUFFER_HEIGHT;
		centerx = 0.697313f;
		centery = -0.090444f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_GM] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.797313f;
		centery = -0.090444f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_GUNDAM] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.897313f;
		centery = -0.090444f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_ZAKU] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		// left
		centerx = -0.270312f;
		centery = 0.705f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.455f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[2] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.205f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[4] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = -0.055f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[6] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		// right
		centerx = 0.516312f;
		centery = 0.705f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[1] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.455f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[3] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.205f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[5] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = -0.055f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_pPlayerRobotRects[7] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	}
}

void CLobbyRoomScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CUIScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

#ifndef ON_NETWORKING
	JoinPlayer(0, 0, L"0번 플레이어", SELECT_CHARACTER_GM);
	JoinPlayer(1, 1, L"1번 플레이어", SELECT_CHARACTER_GM);
	JoinPlayer(2, 2, L"2번 플레이어", SELECT_CHARACTER_GM);
	JoinPlayer(3, 3, L"3번 플레이어", SELECT_CHARACTER_GM);
#endif

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.797187f;
	float centery = -0.375f;
	m_StartButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_StartButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.797187f;
	centery = -0.8425f;
	m_LeaveButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_LeaveButton.Extents = XMFLOAT3(width, height, 1.0f);


	width = 90.0f / FRAME_BUFFER_WIDTH;
	height = 32.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.721875f;
	centery = 0.111111f;
	m_ColonyButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ColonyButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.871875f;
	centery = 0.111111f;
	m_SpaceButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_SpaceButton.Extents = XMFLOAT3(width, height, 1.0f);


	width = 58.0f / FRAME_BUFFER_WIDTH;
	height = 76.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.697313f;
	centery = -0.090444f;
	m_GMButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_GMButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.797313f;
	centery = -0.090444f;
	m_GundamButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_GundamButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.897313f;
	centery = -0.090444f;
	m_ZakuButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ZakuButton.Extents = XMFLOAT3(width, height, 1.0f);


	width = 496.0f / FRAME_BUFFER_WIDTH;
	height = 21.0f / FRAME_BUFFER_HEIGHT;
	centerx = -0.562500f;
	centery = 0.866667f;
	m_TeamRedButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_TeamRedButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.223438f;
	centery = 0.866667f;
	m_TeamBlueButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_TeamBlueButton.Extents = XMFLOAT3(width, height, 1.0f);
}

void CLobbyRoomScene::SetMap(int nMap)
{
	m_nCurrentMap = nMap;
}

void CLobbyRoomScene::ReleaseObjects()
{
	CUIScene::ReleaseObjects();

	for (int i = 0; i < 8; i++)
	{
		if (m_pPlayerRobotRects[i])
			delete m_pPlayerRobotRects[i];
	}

	if (m_MapRect)
	{
		delete m_MapRect;
		m_MapRect = NULL;
	}
}

void CLobbyRoomScene::ReleaseUploadBuffers()
{
	CUIScene::ReleaseUploadBuffers();

	for (int i = 0; i < 8; i++)
	{
		if (m_pPlayerRobotRects[i])
			m_pPlayerRobotRects[i]->ReleaseUploadBuffers();
	}

	if (m_MapRect) m_MapRect->ReleaseUploadBuffers();
}

int CLobbyRoomScene::MouseClick()
{
	if (gClientIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_StartButton))
		{
			return LOBBY_MOUSE_CLICK_ROOM_START;
		}
	}
	if (m_pCursor->CollisionCheck(m_LeaveButton))
	{
		return LOBBY_MOUSE_CLICK_ROOM_LEAVE;
	}

	if (gClientIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_ColonyButton))
		{
			m_nCurrentMap = SCENE_TYPE_COLONY;

			return LOBBY_MOUSE_CLICK_ROOM_SELECT_MAP;
		}

		if (m_pCursor->CollisionCheck(m_SpaceButton))
		{
			m_nCurrentMap = SCENE_TYPE_SPACE;

			return LOBBY_MOUSE_CLICK_ROOM_SELECT_MAP;
		}
	}

	if (m_pCursor->CollisionCheck(m_GMButton))
	{
		CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

		return LOBBY_MOUSE_CLICK_ROOM_SELECT_ROBOT;
	}
	if (m_pCursor->CollisionCheck(m_GundamButton))
	{
		CScene::m_nPlayerRobotType = SELECT_CHARACTER_GUNDAM;

		return LOBBY_MOUSE_CLICK_ROOM_SELECT_ROBOT;
	}
	if (m_pCursor->CollisionCheck(m_ZakuButton))
	{
		CScene::m_nPlayerRobotType = SELECT_CHARACTER_ZAKU;

		return LOBBY_MOUSE_CLICK_ROOM_SELECT_ROBOT;
	}

	if (m_pCursor->CollisionCheck(m_TeamRedButton))
	{
		return LOBBY_MOUSE_CLICK_ROOM_CHANGE_TEAM_RED;
	}

	if (m_pCursor->CollisionCheck(m_TeamBlueButton))
	{
		return LOBBY_MOUSE_CLICK_ROOM_CHANGE_TEAM_BLUE;
	}

	return 0;
}

void CLobbyRoomScene::CheckCollision()
{
	if (m_pCursor->CollisionCheck(m_StartButton)) m_bHLStartButton = true;
	else m_bHLStartButton = false;

	if (m_pCursor->CollisionCheck(m_LeaveButton)) m_bHLLeaveButton = true;
	else m_bHLLeaveButton = false;

	if (gClientIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_ColonyButton)) m_bHLColonyButton = true;
		else m_bHLColonyButton = false;

		if (m_pCursor->CollisionCheck(m_SpaceButton)) m_bHLSpaceButton = true;
		else m_bHLSpaceButton = false;
	}

	if (m_pCursor->CollisionCheck(m_GMButton)) m_bHLGMButton = true;
	else m_bHLGMButton = false;

	if (m_pCursor->CollisionCheck(m_GundamButton)) m_bHLGundamButton = true;
	else m_bHLGundamButton = false;

	if (m_pCursor->CollisionCheck(m_ZakuButton)) m_bHLZakuButton = true;
	else m_bHLZakuButton = false;

	if (m_pCursor->CollisionCheck(m_TeamRedButton)) m_bHLTeamRedButton = true;
	else m_bHLTeamRedButton = false;

	if (m_pCursor->CollisionCheck(m_TeamBlueButton)) m_bHLTeamBlueButton = true;
	else m_bHLTeamBlueButton = false;
}

void CLobbyRoomScene::StartScene(bool bBGMStop)
{
	//gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

void CLobbyRoomScene::SetClientIndex(int nIndex, int nSlot)
{
	gClientIndex = nIndex;
	JoinPlayer(nIndex, nSlot, CScene::m_pwstrMyName, SELECT_CHARACTER_GM);

	CScene::m_nMyTeam = (nSlot % 2) == 0 ? TEAM_TYPE::TEAM_TYPE_RED : TEAM_TYPE::TEAM_TYPE_BLUE;
}

void CLobbyRoomScene::ChangeSelectRobot(int nIndex, int nRobotType)
{
	m_pPlayerInfos[nIndex].m_nRobotType = nRobotType;
}

XMFLOAT2 CLobbyRoomScene::GetPlayerTextPosition(int nSlotIndex)
{
	XMFLOAT2 xmf2Pos;
	xmf2Pos.y = 0.74f;

	if (nSlotIndex % 2 == 0)
		xmf2Pos.x = -0.916875f;
	else
		xmf2Pos.x = -0.130937f;

	xmf2Pos.y += ((int)(nSlotIndex / 2)) * -0.255f;

	return xmf2Pos;
}

void CLobbyRoomScene::JoinPlayer(int nIndex, int nSlot, const wchar_t *pstrPlayerName, int nRobotType)
{
	m_pPlayerInfos[nIndex].m_nSlot = nSlot;
	m_pPlayerInfos[nIndex].m_nRobotType = nRobotType;
	m_pPlayerInfos[nIndex].m_bUsed = true;

	XMFLOAT2 xmf2Pos = GetPlayerTextPosition(nSlot);

	XMFLOAT4 xmf4Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	if (gClientIndex == nIndex)
	{
		xmf4Color.x = 1.0f;
		xmf4Color.z = 0.0f;
	}

	m_pPlayerInfos[nIndex].m_pTextObject = m_pFont->SetText(pstrPlayerName, XMFLOAT2(0.0f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), xmf4Color, LEFT_ALIGN);
	m_pPlayerInfos[nIndex].m_pTextObject->SetPosition(xmf2Pos);
}

void CLobbyRoomScene::ChangeSlot(int nIndex, int nChangeSlot)
{
	XMFLOAT2 xmf2Pos = GetPlayerTextPosition(nChangeSlot);

	m_pPlayerInfos[nIndex].m_pTextObject->SetPosition(xmf2Pos);
	m_pPlayerInfos[nIndex].m_nSlot = nChangeSlot;

	if(gClientIndex == nIndex) CScene::m_nMyTeam = (nChangeSlot % 2) == 0 ? TEAM_TYPE::TEAM_TYPE_RED : TEAM_TYPE::TEAM_TYPE_BLUE;
}

void CLobbyRoomScene::LeavePlayer(int nIndex)
{
	m_pPlayerInfos[nIndex].m_bUsed = false;
	m_pPlayerInfos[nIndex].m_pTextObject->Release();
}

void CLobbyRoomScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	if (m_nCurrentMap == SCENE_TYPE_COLONY)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_COLONY]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nCurrentMap == SCENE_TYPE_SPACE)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_MAP_SPACE]->UpdateShaderVariables(pd3dCommandList);
	m_MapRect->Render(pd3dCommandList, 0);

	if (gClientIndex == 0)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER]->UpdateShaderVariables(pd3dCommandList);

	m_ppUIRects[LOBBY_ROOM_UI_RECT_BASE]->Render(pd3dCommandList, 0);

	CScene::RenderUI(pd3dCommandList);

	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	if (m_bHLTeamRedButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED_HL]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_RED]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_TEAM_RED]->Render(pd3dCommandList, 0);

	if (m_bHLTeamBlueButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE_HL]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_TEAM_BLUE]->Render(pd3dCommandList, 0);

	if (gClientIndex == 0)
	{
		if (m_bHLStartButton)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_START_BUTTON]->Render(pd3dCommandList, 0);
	}

	if (m_bHLLeaveButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_LEAVE_BUTTON]->Render(pd3dCommandList, 0);

	if (m_bHLColonyButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nCurrentMap == SCENE_TYPE_COLONY)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_COLONY]->Render(pd3dCommandList, 0);

	if (m_bHLSpaceButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nCurrentMap == SCENE_TYPE_SPACE)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_SPACE]->Render(pd3dCommandList, 0);

	if (m_bHLGMButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nPlayerRobotType == SELECT_CHARACTER_GM)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_GM]->Render(pd3dCommandList, 0);

	if (m_bHLGundamButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nPlayerRobotType == SELECT_CHARACTER_GUNDAM)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_GUNDAM]->Render(pd3dCommandList, 0);

	if (m_bHLZakuButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nPlayerRobotType == SELECT_CHARACTER_ZAKU)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_ZAKU]->Render(pd3dCommandList, 0);

	for(int i = 0 ; i < 8; i++)
	{
		if (!m_pPlayerInfos[i].m_bUsed) continue;

		if (m_pPlayerInfos[i].m_nRobotType == SELECT_CHARACTER_GM)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->UpdateShaderVariables(pd3dCommandList);
		else if (m_pPlayerInfos[i].m_nRobotType == SELECT_CHARACTER_GUNDAM)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->UpdateShaderVariables(pd3dCommandList);
		else if (m_pPlayerInfos[i].m_nRobotType == SELECT_CHARACTER_ZAKU)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_ZAKU]->UpdateShaderVariables(pd3dCommandList);

		m_pPlayerRobotRects[m_pPlayerInfos[i].m_nSlot]->Render(pd3dCommandList, 0);
	}

	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

void CLobbyRoomScene::GetTeamsInfo(int nTeam, std::vector<int> &vnIndices, std::vector<int> &vnEnemyIndices, std::vector<wchar_t*> &vpwstrNames)
{
	std::map<int, int> slotindex;
	std::map<int, int> enemyIndex;

	for (int i = 0; i < 8; i++)
	{
		if (!m_pPlayerInfos[i].m_bUsed) continue;
		if (i == gClientIndex) continue;

		if (nTeam == TEAM_TYPE::TEAM_TYPE_RED)
		{
			if (m_pPlayerInfos[i].m_nSlot % 2 == 0)
			{
				slotindex[m_pPlayerInfos[i].m_nSlot] = i;
			}
			else enemyIndex[m_pPlayerInfos[i].m_nSlot] = i;
		}
		else if (nTeam == TEAM_TYPE::TEAM_TYPE_BLUE)
		{
			if (m_pPlayerInfos[i].m_nSlot % 2 == 1)
			{
				slotindex[m_pPlayerInfos[i].m_nSlot] = i;
			}
			else enemyIndex[m_pPlayerInfos[i].m_nSlot] = i;
		}
	}

	for (auto iter : slotindex)
	{
		int nIndex = iter.second;

		vnIndices.emplace_back(nIndex);
		vpwstrNames.emplace_back(m_pPlayerInfos[nIndex].m_pTextObject->GetText());
	}

	for (auto iter : enemyIndex)
	{
		int nIndex = iter.second;
		vnEnemyIndices.emplace_back(nIndex);
	}

}

void CLobbyRoomScene::EndScene()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBattleScene::CBattleScene() : CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
		m_pObjects[i] = NULL;

	m_xmf4x4PrevViewProjection = Matrix4x4::Identity();
	m_xmf4ScreenColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

CBattleScene::~CBattleScene()
{
}

int CBattleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_bAction) return 0;
	if (m_bGameEnd) return 0;

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		if (::GetCapture() == hWnd)
		{
			if (!m_LButtonDown) m_pPlayer->PrepareAttack(m_pPlayer->GetRHWeapon());

			m_LButtonDown = TRUE;
			m_pPlayer->Attack(m_pPlayer->GetRHWeapon());
		}
		break;
	case WM_LBUTTONUP:
	{
		m_LButtonDown = FALSE;
		m_pPlayer->LButtonUp();
		break;
	}
	case WM_RBUTTONDOWN:
		m_pPlayer->TakeAim();
		break;
	case WM_RBUTTONUP:
		m_pPlayer->RButtonUp();
		break;
	default:
		break;
	}

	return 0;
}

int CBattleScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene::OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	if (!m_bAction) return 0;

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_SHIFT:
			m_pPlayer->ShiftUp();
			break;
		case VK_SPACE:
			m_pPlayer->SpaceUp();
			break;
		case 'ㅍ':
		case 'V':
			m_pPlayer->VUp();
			break;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		if (m_bGameEnd)
		{
			if(wParam == VK_RETURN)
				return LOBBY_MOVE;
		}
		else
		{
			switch (wParam)
			{
			case '1':
				m_pPlayer->ChangeWeapon(0);
				break;
			case '2':
				m_pPlayer->ChangeWeapon(1);
				break;
			case '3':
				m_pPlayer->ChangeWeapon(2);
				break;
			case 'R':
			case 'ㄱ':
				m_pPlayer->Reload(m_pPlayer->GetRHWeapon());
				break;
			case VK_SPACE:
				m_pPlayer->ActivationBooster(BOOSTER_TYPE_UP);
				break;
			case 'ㅍ':
			case 'V':
				m_pPlayer->ActivationBooster(BOOSTER_TYPE_DOWN);
				break;
			case VK_SHIFT:
				m_pPlayer->ActivationDash();
				break;
			case VK_F1:
				m_bRenderEdge = !m_bRenderEdge;
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

void CBattleScene::ProcessInput(float fTimeElapsed)
{
	if (!m_bAction) return;

	static UCHAR pKeyBuffer[256];

	if (m_pPlayer) m_pPlayer->DeactiveMoving();

	if (::GetKeyboardState(pKeyBuffer) || m_pPlayer->IsDash())
	{
		if (m_pPlayer)
		{
			ULONG dwDirection = 0;

			if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;

			m_pPlayer->Move(dwDirection, m_pPlayer->GetMovingSpeed() * fTimeElapsed);
		}
	}
}

void CBattleScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);
	CreateRtvAndDsvDescriptorHeaps(pd3dDevice);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	BuildLightsAndMaterials();

	// Objects
	m_nShaders = SHADER_INDEX;
	m_ppShaders = new CShader*[m_nShaders];
	ZeroMemory(m_ppShaders, sizeof(CShader*) * m_nShaders);

	/* 그룹 1 [ Standard Shader ] */
	CStandardObjectsShader *pStandardObjectsShader = new CStandardObjectsShader();
	pStandardObjectsShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pStandardObjectsShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS] = pStandardObjectsShader;

	/* 그룹 2 [ Instancing Shader ] */
	BuildObstacleObjetcs(pd3dDevice, pd3dCommandList, pRepository);

	/* 그룹 3 [ Skinned Animation Shader ] */
	CRobotObjectsShader *pSkinnedRobotShader = new CRobotObjectsShader();
	pSkinnedRobotShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pSkinnedRobotShader->Initialize(pd3dDevice, pd3dCommandList, pRepository, m_pd3dGraphicsRootSignature);
	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS] = pSkinnedRobotShader;

	// Effects
	m_nEffectShaders = EFFECT_SHADER_INDEX;
	m_ppEffectShaders = new CEffectShader*[m_nEffectShaders];

	// 그룹 1 [ Text Effect Shader ]
	CTextEffectShader *pTextEffectShader = new CTextEffectShader();
	pTextEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTextEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS] = pTextEffectShader;

	// 그룹 2 [ Timed Effect Shader ]
	CTimedEffectShader *pTimedEffectShader = new CTimedEffectShader();
	pTimedEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTimedEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS] = pTimedEffectShader;

	// 그룹 3 [ Sprite Shader ]
	CSpriteShader *pSpriteShader = new CSpriteShader();
	pSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS] = pSpriteShader;

	// 그룹 4 [ Laser Effect Shader ]
	CLaserEffectShader *pLaserEffectShader = new CLaserEffectShader();
	pLaserEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pLaserEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS] = pLaserEffectShader;

	// 그룹 5 [ Follow Effect Shader ]
	CFollowSpriteShader *pFollowSpriteShader = new CFollowSpriteShader();
	pFollowSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pFollowSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_FOLLOW_SPRITE_EFFECTS] = pFollowSpriteShader;

	// Particle
	m_pParticleShader = new CParticleShader();
	m_pParticleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pParticleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	////
	// Wire
	m_pWireShader = new CWireShader();
	m_pWireShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	// Terrain
	BuildTerrain(pd3dDevice, pd3dCommandList);

	// SkyBox
	BuildSkybox(pd3dDevice, pd3dCommandList);

	// Weapons
	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", NULL, NULL);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", NULL, NULL);
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", NULL, NULL);
	m_pSaber = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/Saber.bin", NULL, NULL);
	m_pTomahawk = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/Tomahawk.bin", NULL, NULL);
	m_pBeamRifle = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BeamRifle.bin", NULL, NULL);
	m_pBeamSniper = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BeamSniper.bin", NULL, NULL);


	m_pPostProcessingShader = new CPostProcessingShader();
	m_pPostProcessingShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pTestShader = new CTestShader();
	m_pTestShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pComputeShader = new CComputeShader();
	m_pComputeShader->CreateShader(pd3dDevice, m_pd3dComputeRootSignature);
}

void CBattleScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);
	CreateOffScreenTextures(pd3dDevice);

	m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetRightNozzleFrame());
	m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetLeftNozzleFrame());

	m_ppEffectShaders[INDEX_SHADER_FOLLOW_SPRITE_EFFECTS]->SetFollowObject(FOLLOW_SPRITE_EFFECT_INDEX_BOOSTER, m_pPlayer, m_pPlayer->GetRightNozzleFrame());
	m_ppEffectShaders[INDEX_SHADER_FOLLOW_SPRITE_EFFECTS]->SetFollowObject(FOLLOW_SPRITE_EFFECT_INDEX_BOOSTER, m_pPlayer, m_pPlayer->GetLeftNozzleFrame());

	if (m_pPlayer)
	{
		m_pPlayer->SetScene(this);
	}

	switch (CScene::m_nPlayerRobotType)
	{
	case SELECT_CHARACTER_GM:
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_SABER);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_GM_GUN);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_BEAM_SNIPER);
		break;
	case SELECT_CHARACTER_GUNDAM: // 빔사벨, 빔라이플, 바주카
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_SABER);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_BEAM_RIFLE);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_BAZOOKA);
		break;
	case SELECT_CHARACTER_ZAKU: // 토마호크, 머신건, 바주카
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_TOMAHAWK);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_MACHINEGUN);
		AddWeaponToPlayer(pd3dDevice, pd3dCommandList, WEAPON_TYPE_OF_BAZOOKA);
		break;
	}

#ifndef ON_NETWORKING
	CRobotObject *pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 0.0f, 50.0f));

	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS]->InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_GM, true, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS]);

	pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(50.0f, 0.0f, 0.0f));

	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS]->InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_ZAKU, true, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS]);

	pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(-50.0f, 0.0f, 0.0f));

	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS]->InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_GUNDAM, true, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS]);

	////
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GM_GUN, 50);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, 20);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, 300);
#endif

	CUserInterface *pUserInterface = new CUserInterface();
	pUserInterface->SetFont(m_pFont);
	pUserInterface->Initialize(pd3dDevice, pd3dCommandList, NULL);
	pUserInterface->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pUserInterface->SetPlayer(m_pPlayer);
	pUserInterface->SetAmmoText(0);
	pUserInterface->SetScene(this);

	for (int i = 0; i < m_vTeamIndex.size(); i++)
	{
		pUserInterface->SetTeamInfo(&m_pObjects[m_vTeamIndex[i]], m_vwstrTeamName[i].c_str());
	}
	pUserInterface->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pUserInterface = pUserInterface;


	//
	XMFLOAT2 xmf2Center = ::CalculateCenter(0.63f, 0.93f, 0.93f, 0.4f, true);
	XMFLOAT2 xmf2Size = ::CalculateSize(0.63f, 0.93f, 0.93f, 0.4f, true);

	CMinimapShader *pMinimapShader = new CMinimapShader(pd3dDevice, pd3dCommandList);
	pMinimapShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	for (int i = 0; i < m_vTeamIndex.size(); i++)
	{
		pMinimapShader->SetTeamInfo(&m_pObjects[m_vTeamIndex[i]]);
	}
	for (int i = 0; i < m_vEnemyIndex.size(); i++)
	{
		pMinimapShader->SetEnemyInfo(&m_pObjects[m_vEnemyIndex[i]]);
	}
	pMinimapShader->SetRobotCount();
	pMinimapShader->Initialize(pd3dDevice, pd3dCommandList, xmf2Center.x, xmf2Center.y, xmf2Size.x, xmf2Size.y);
	pMinimapShader->SetPlayer(m_pPlayer);

	m_pMinimapShader = pMinimapShader;
	//

	m_pRedScoreText = m_pFont->SetText(L"100", XMFLOAT2(-0.02f, 0.88f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.0f, 0.9f), RIGHT_ALIGN);
	m_pBlueScoreText = m_pFont->SetText(L"100", XMFLOAT2(0.02f, 0.88f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.5f, 0.9f), LEFT_ALIGN);

	CreateEnvironmentMap(pd3dDevice);
	CreateCubeMapCamera(pd3dDevice, pd3dCommandList);
	CreateShadowMap(pd3dDevice, pd3dCommandList, 4096 * 2, 4096 * 2);

	if (m_vTeamIndex.size() > 0)
	{
		CreateNameTextures(pd3dDevice, pd3dCommandList);
		std::cout << "네임텍스쳐 완료\n";
	}
#ifndef ON_NETWORKING
	m_pUserInterface->BattleNotifyStart();
#endif
}

void CBattleScene::ReleaseObjects()
{
	CScene::ReleaseObjects();
	
	if (m_pLights)
	{
		delete m_pLights;
		m_pLights = NULL;
	}

	if (m_pSkyBox)
	{
		m_pSkyBox->ReleaseShaderVariables();
		delete m_pSkyBox;

		m_pSkyBox = NULL;
	}

	if (m_pTerrain)
	{
		m_pTerrain->ReleaseShaderVariables();
		delete m_pTerrain;

		m_pTerrain = NULL;
	}

	if (m_pWireShader)
	{
		m_pWireShader->ReleaseShaderVariables();
		m_pWireShader->ReleaseObjects();
		delete m_pWireShader;

		m_pWireShader = NULL;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
			{
				m_ppShaders[i]->ReleaseShaderVariables();
				m_ppShaders[i]->ReleaseObjects();
				delete m_ppShaders[i];
				m_ppShaders[i] = NULL;
			}
		}

		delete[] m_ppShaders;
		m_ppShaders = NULL;
	}

	if (m_ppEffectShaders)
	{
		for (int i = 0; i < m_nEffectShaders; i++)
		{
			if (m_ppEffectShaders[i])
			{
				m_ppEffectShaders[i]->ReleaseShaderVariables();
				m_ppEffectShaders[i]->ReleaseObjects();
				delete m_ppEffectShaders[i];
				m_ppEffectShaders[i] = NULL;
			}
		}
		delete[] m_ppEffectShaders;
		m_ppEffectShaders = NULL;
	}

	if (m_pParticleShader)
	{
		m_pParticleShader->ReleaseShaderVariables();
		m_pParticleShader->ReleaseObjects();
		delete m_pParticleShader;
		m_pParticleShader = NULL;
	}

	if (m_pUserInterface)
	{
		m_pUserInterface->ReleaseShaderVariables();
		m_pUserInterface->ReleaseObjects();
		delete m_pUserInterface;

		m_pUserInterface = NULL;
	}

	if (m_pPostProcessingShader)
	{
		m_pPostProcessingShader->ReleaseShaderVariables();
		m_pPostProcessingShader->ReleaseObjects();
		delete m_pPostProcessingShader;
		m_pPostProcessingShader = NULL;
	}

	if (m_pTestShader)
	{
		m_pTestShader->ReleaseShaderVariables();
		m_pTestShader->ReleaseObjects();
		delete m_pTestShader;
		m_pTestShader = NULL;
	}

	if (m_pComputeShader)
	{
		m_pComputeShader->ReleaseObjects();
		delete m_pComputeShader;
		m_pComputeShader = NULL;
	}


	for (int i = 0; i < 6; i++)
	{
		if (m_pCubeMapCamera[i])
		{
			m_pCubeMapCamera[i]->ReleaseShaderVariables();
			delete m_pCubeMapCamera[i];

			m_pCubeMapCamera[i] = NULL;
		}
	}

	if (m_pLightCamera)
	{
		m_pLightCamera->ReleaseShaderVariables();
		delete m_pLightCamera;

		m_pLightCamera = NULL;
	}

	if (m_pMinimapShader)
	{
		m_pMinimapShader->ReleaseShaderVariables();
		m_pMinimapShader->ReleaseObjects();
		delete m_pMinimapShader;

		m_pMinimapShader = NULL;
	}

	if (m_pd3dOffScreenTexture)
	{
		m_pd3dOffScreenTexture->Release();
		m_pd3dOffScreenTexture = NULL;
	}
	if (m_pd3dGlowScreenTexture)
	{
		m_pd3dGlowScreenTexture->Release();
		m_pd3dGlowScreenTexture = NULL;
	}
	if (m_pd3dDepthStencilBuffer)
	{
		m_pd3dDepthStencilBuffer->Release();
		m_pd3dDepthStencilBuffer = NULL;
	}
	if (m_pd3dMotionBlurScreenTexture)
	{
		m_pd3dMotionBlurScreenTexture->Release();
		m_pd3dMotionBlurScreenTexture = NULL;
	}
	if (m_pd3dTempTexture)
	{
		m_pd3dTempTexture->Release();
		m_pd3dTempTexture = NULL;
	}
	if (m_pd3dMaskTexture)
	{
		m_pd3dMaskTexture->Release();
		m_pd3dMaskTexture = NULL;
	}
	if (m_pd3dScreenNormalTexture)
	{
		m_pd3dScreenNormalTexture->Release();
		m_pd3dScreenNormalTexture = NULL;
	}

	if (m_pd3dEnvirCube)
	{
		m_pd3dEnvirCube->Release();
		m_pd3dEnvirCube = NULL;
	}

	if (m_pd3dEnvirCubeDSBuffer)
	{
		m_pd3dEnvirCubeDSBuffer->Release();
		m_pd3dEnvirCubeDSBuffer = NULL;
	}

	if (m_pd3dShadowMap)
	{
		m_pd3dShadowMap->Release();
		m_pd3dShadowMap = NULL;
	}

	if (m_pd3dRtvDescriptorHeap)
	{
		m_pd3dRtvDescriptorHeap->Release();
		m_pd3dRtvDescriptorHeap = NULL;
	}

	if (m_pd3dDsvDescriptorHeap)
	{
		m_pd3dDsvDescriptorHeap->Release();
		m_pd3dDsvDescriptorHeap = NULL;
	}
}

void CBattleScene::Alert()
{
	m_bAlert = true;
}

void CBattleScene::ProcessAlert(float fElapsedTime)
{
	m_fAlertColor += (m_fMulCalcAlertColor * ALERT_SPEED * fElapsedTime);

	if (m_fAlertColor < 0.0f)
	{
		m_fAlertColor = 0.0f;
		m_fMulCalcAlertColor = 1.0f;
	}
	else if (m_fAlertColor > 1.0f)
	{
		m_fAlertColor = 1.0f;
		m_fMulCalcAlertColor = -1.0f;

		m_fAlertCount++;

		if (m_fAlertCount != ALERT_COUNT)
			gFmodSound.PlayFMODSound(gFmodSound.m_pSoundAlert);
	}

	m_xmf4ScreenColor.y = m_xmf4ScreenColor.z = m_fAlertColor;

	if (m_fAlertCount == ALERT_COUNT)
	{
		m_bAlert = false;
		m_fAlertCount = 0;
		m_fMulCalcAlertColor = 1.0f;
		m_xmf4ScreenColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_fAlertColor = 1.0f;
	}
}

void CBattleScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pPlayer) m_pPlayer->ApplyGravity(m_fGravAcc, fTimeElapsed);
	if (m_bAlert) ProcessAlert(fTimeElapsed);

	if (m_pUserInterface)
	{
		m_pUserInterface->AnimateObjects(fTimeElapsed, pCamera);
	}

	if (m_pPlayer) m_pPlayer->Update(fTimeElapsed);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
	}

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->AnimateObjects(fTimeElapsed);

	if (m_pTerrain) m_pTerrain->Animate(fTimeElapsed, pCamera);

	if (m_pLights)
	{
		m_pLights->m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights->m_pLights->m_xmf3Direction = m_pPlayer->GetLook();
	}

	m_fFPS = fTimeElapsed;

	CScene::AnimateObjects(fTimeElapsed, pCamera);
}

void CBattleScene::AddWeaponToPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nType)
{
	CWeapon *pWeapon = NULL;

	switch (nType)
	{
	case WEAPON_TYPE_OF_GM_GUN:
		pWeapon = new CGimGun();
		pWeapon->SetModel(m_pGimGun);
		((CGun*)pWeapon)->SetBullet(m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS], m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS],  NULL);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_BAZOOKA:
		pWeapon = new CBazooka();
		pWeapon->SetModel(m_pBazooka);
		((CGun*)pWeapon)->SetBullet(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS], STANDARD_OBJECT_INDEX_BZK_BULLET);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_MACHINEGUN:
		pWeapon = new CMachineGun();
		pWeapon->SetModel(m_pMachineGun);
		((CGun*)pWeapon)->SetBullet(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS], STANDARD_OBJECT_INDEX_MG_BULLET);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_BEAM_RIFLE:
		pWeapon = new CBeamRifle();
		pWeapon->SetModel(m_pBeamRifle);
		((CGun*)pWeapon)->SetBullet(m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS], m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS], NULL);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_BEAM_SNIPER:
		pWeapon = new CBeamSniper();
		pWeapon->SetModel(m_pBeamSniper);
		((CGun*)pWeapon)->SetBullet(m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS], m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS], NULL);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_SABER:
		pWeapon = new CSaber();
		pWeapon->SetModel(m_pSaber);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	case WEAPON_TYPE_OF_TOMAHAWK:
		pWeapon = new CTomahawk();
		pWeapon->SetModel(m_pTomahawk);
		m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, pWeapon);
		break;
	}

	pWeapon->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CBattleScene::CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = SCENE_RTV_DESCRIPTOR_HEAP_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);

	m_d3dRtvCPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dRtvGPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	d3dDescriptorHeapDesc.NumDescriptors = SCENE_DSV_DESCRIPTOR_HEAP_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);

	m_d3dDsvCPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dDsvGPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

void CBattleScene::CreateRenderTargetView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_RTV_DIMENSION d3dRtvDimension, int nViews, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dDesc;
	d3dDesc.ViewDimension = d3dRtvDimension;
	d3dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	for (int i = 0; i < nViews; i++)
	{
		switch (d3dRtvDimension)
		{
		case D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D:
			d3dDesc.Texture2D.MipSlice = 0;
			d3dDesc.Texture2D.PlaneSlice = 0;
			break;
		case D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
			d3dDesc.Texture2DArray.MipSlice = 0;
			d3dDesc.Texture2DArray.PlaneSlice = 0;
			d3dDesc.Texture2DArray.FirstArraySlice = i;
			d3dDesc.Texture2DArray.ArraySize = 1;
			break;
		}

		pd3dDevice->CreateRenderTargetView(pd3dResource, &d3dDesc, m_d3dRtvCPUDesciptorStartHandle);
		pd3dSaveCPUHandle[i] = m_d3dRtvCPUDesciptorStartHandle;

		m_d3dRtvCPUDesciptorStartHandle.ptr += ::gnRtvDescriptorIncrementSize;
		m_d3dRtvGPUDesciptorStartHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CBattleScene::CreateDepthStencilView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	pd3dDevice->CreateDepthStencilView(pd3dResource, &d3dDepthStencilViewDesc, m_d3dDsvCPUDesciptorStartHandle);
	(*pd3dSaveCPUHandle) = m_d3dDsvCPUDesciptorStartHandle;

	m_d3dDsvCPUDesciptorStartHandle.ptr += ::gnDsvDescriptorIncrementSize;
	m_d3dDsvGPUDesciptorStartHandle.ptr += ::gnDsvDescriptorIncrementSize;
}

void CBattleScene::CreateNameTextures(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHeapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dHeapGPUHandle;

	ID3D12DescriptorHeap *pd3dRTVHeap;
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = (int)m_vwstrTeamName.size();
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&pd3dRTVHeap);

	if (pd3dRTVHeap != nullptr)
	{
		d3dHeapCPUHandle = pd3dRTVHeap->GetCPUDescriptorHandleForHeapStart();
		d3dHeapGPUHandle = pd3dRTVHeap->GetGPUDescriptorHandleForHeapStart();

		std::vector<CTextObject*>					vpTeamTextObject;
		std::vector<ID3D12Resource*>				vpd3dTeamNameTexture;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>	vd3dTeamTextureRTVCPUHandle;
		std::vector<CRect*>							vpTeamNameRect;
		std::vector<D3D12_VIEWPORT>					vd3dViewport;
		std::vector<D3D12_RECT>						vd3dScissorRect;

		for (int i = 0; i < m_vwstrTeamName.size(); i++)
		{
			int W, H;
			CTextObject *pTemp = m_pFont->Set3DText(W, H, m_vwstrTeamName[i].c_str(), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
			vpTeamTextObject.emplace_back(pTemp);

			CRect *pRect = new CRect(pd3dDevice, pd3dCommandList, XMFLOAT2(0.0f, 0.0f), XMFLOAT2(float(W) * 0.04f, float(H) * 0.04f));
			D3D12_VIEWPORT viewport = { 0, 0, float(W), float(H), 0.0f, 1.0f };
			vd3dViewport.emplace_back(viewport);

			D3D12_RECT scissorRect = { 0, 0, W, H };
			vd3dScissorRect.emplace_back(scissorRect);

			ID3D12Resource *pTextTexture;
			CreateNameTexture(pd3dDevice, pd3dCommandList, &pTextTexture, W, H);
			vpd3dTeamNameTexture.emplace_back(pTextTexture);

			D3D12_RENDER_TARGET_VIEW_DESC d3dDesc;
			d3dDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			d3dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			d3dDesc.Texture2D.MipSlice = 0;
			d3dDesc.Texture2D.PlaneSlice = 0;

			pd3dDevice->CreateRenderTargetView(pTextTexture, &d3dDesc, d3dHeapCPUHandle);
			vd3dTeamTextureRTVCPUHandle.emplace_back(d3dHeapCPUHandle);

			d3dHeapCPUHandle.ptr += ::gnRtvDescriptorIncrementSize;
			d3dHeapGPUHandle.ptr += ::gnRtvDescriptorIncrementSize;

			m_pUserInterface->SetTeamNameTexture(pd3dDevice, pTextTexture, pRect);
		}

		CScene::SetDescHeapsAndGraphicsRootSignature(pd3dCommandList);

		for (int i = 0; i < vpd3dTeamNameTexture.size(); i++)
		{
			pd3dCommandList->OMSetRenderTargets(1, &vd3dTeamTextureRTVCPUHandle[i], false, NULL);

			pd3dCommandList->RSSetViewports(1, &vd3dViewport[i]);
			pd3dCommandList->RSSetScissorRects(1, &vd3dScissorRect[i]);

			m_pFontShader->OnPrepareRender(pd3dCommandList);

			m_pFont->OnPrepareRender(pd3dCommandList);
			vpTeamTextObject[i]->Render(pd3dCommandList);

			vpTeamTextObject[i]->Release();
			TransitionResourceState(pd3dCommandList, vpd3dTeamNameTexture[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}

		pd3dRTVHeap->Release();
	}
	else
	{
		std::cout << "RTV힙이 생성되지 않음\n";
	}
}

void CBattleScene::CreateNameTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource **pd3dResource, int nWidth, int nHeight)
{
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nWidth;
	d3dResourceDesc.Height = nHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &d3dClear, __uuidof(ID3D12Resource), (void **)pd3dResource);
}

void CBattleScene::CreateEnvironmentMap(ID3D12Device *pd3dDevice)
{
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = CUBE_MAP_WIDTH;
	d3dResourceDesc.Height = CUBE_MAP_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 6;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCube);

	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	d3dClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClear.DepthStencil.Depth = 1.0f;
	d3dClear.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCubeDSBuffer);

	CreateRtvDsvSrvEnvironmentMap(pd3dDevice);
}

void CBattleScene::CreateRtvDsvSrvEnvironmentMap(ID3D12Device *pd3dDevice)
{
	CreateRenderTargetView(pd3dDevice, m_pd3dEnvirCube, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 6, m_d3dRrvEnvirCubeMapCPUHandle);
	CreateDepthStencilView(pd3dDevice, m_pd3dEnvirCubeDSBuffer, &m_d3dDsvEnvirCubeMapCPUHandle);
	m_d3dSrvEnvirCubeMapGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dEnvirCube, RESOURCE_TEXTURE_CUBE);
}

void CBattleScene::CreateCubeMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 xmf3Looks[6] = {
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f)
	};

	XMFLOAT3 xmf3Ups[6] = {
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	};

	for (int i = 0; i < 6; i++)
	{
		XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks[i], xmf3Ups[i], true);

		m_pCubeMapCamera[i] = new CCamera();

		m_pCubeMapCamera[i]->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCubeMapCamera[i]->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCubeMapCamera[i]->SetRight(xmf3Right);
		m_pCubeMapCamera[i]->SetUp(xmf3Ups[i]);
		m_pCubeMapCamera[i]->SetLook(xmf3Looks[i]);
		m_pCubeMapCamera[i]->SetViewPort(0.0f, 0.0f, float(CUBE_MAP_WIDTH), float(CUBE_MAP_HEIGHT));
		m_pCubeMapCamera[i]->SetScissorRect(0, 0, CUBE_MAP_WIDTH, CUBE_MAP_HEIGHT);

		m_pCubeMapCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}

void CBattleScene::CreateShadowMap(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nWidth;
	d3dResourceDesc.Height = nHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE d3dClear;
	d3dClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClear.DepthStencil.Depth = 1.0f;
	d3dClear.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dShadowMap);

	CreateDsvSrvShadowMap(pd3dDevice);
	CreateLightCamera(pd3dDevice, pd3dCommandList, nWidth, nHeight);
}

void CBattleScene::CreateDsvSrvShadowMap(ID3D12Device *pd3dDevice)
{
	CreateDepthStencilView(pd3dDevice, m_pd3dShadowMap, &m_d3dDsvShadowMapCPUHandle);
	m_d3dSrvShadowMapGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dShadowMap, RESOURCE_TEXTURE2D, true);
}

void CBattleScene::CreateOffScreenTextures(ID3D12Device *pd3dDevice)
{
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(d3dHeapProperties));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_FLAGS d3dHeapFlags = D3D12_HEAP_FLAG_NONE;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(d3dResourceDesc));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = FRAME_BUFFER_WIDTH;
	d3dResourceDesc.Height = FRAME_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_CLEAR_VALUE d3dClearValue;
	::ZeroMemory(&d3dClearValue, sizeof(d3dClearValue));
	d3dClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dClearValue.Color[0] = 0.0f;
	d3dClearValue.Color[1] = 0.0f;
	d3dClearValue.Color[2] = 0.0f;
	d3dClearValue.Color[3] = 1.0f;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dOffScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dGlowScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dScreenNormalTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dTempTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dMotionBlurScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dMaskTexture);

	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	CreateRtvDsvSrvUavOffScreens(pd3dDevice);
}

void CBattleScene::CreateRtvDsvSrvUavOffScreens(ID3D12Device *pd3dDevice)
{
	CreateRenderTargetView(pd3dDevice, m_pd3dOffScreenTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvOffScreenCPUHandle);
	CreateDepthStencilView(pd3dDevice, m_pd3dDepthStencilBuffer, &m_d3dDsvOffScreenCPUHandle);
	m_d3dSrvOffScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dOffScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavOffScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dOffScreenTexture);
	m_d3dSrvDepthStencilGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dDepthStencilBuffer, RESOURCE_TEXTURE2D, true);

	CreateRenderTargetView(pd3dDevice, m_pd3dGlowScreenTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvGlowScreenCPUHandle);
	m_d3dSrvGlowScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dGlowScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavGlowScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dGlowScreenTexture);

	m_d3dSrvMotionBlurScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dMotionBlurScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavMotionBlurScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dMotionBlurScreenTexture);

	m_d3dSrvTempTextureGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dTempTexture, RESOURCE_TEXTURE2D);
	m_d3dUavTempTextureGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dTempTexture);

	CreateRenderTargetView(pd3dDevice, m_pd3dScreenNormalTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvScreenNormalCPUHandle);
	m_d3dSrvScreenNormalGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dScreenNormalTexture, RESOURCE_TEXTURE2D);

	CreateRenderTargetView(pd3dDevice, m_pd3dMaskTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvMaskTextureCPUHandle);
	m_d3dSrvMaskTextureGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dMaskTexture, RESOURCE_TEXTURE2D);
}

void CBattleScene::RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pMainObject)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	for(int i = 0; i < 6; i++)
		pd3dCommandList->ClearRenderTargetView(m_d3dRrvEnvirCubeMapCPUHandle[i], Colors::Black, 0, NULL);

	for (int i = 0; i < 6; i++)
	{
		m_pCubeMapCamera[i]->SetPosition(Vector3::Add(pMainObject->GetPosition(), XMFLOAT3(0.0f, 5.0f, 0.0f)));
		m_pCubeMapCamera[i]->GenerateViewMatrix();
		m_pCubeMapCamera[i]->OnPrepareRender(pd3dCommandList);

		pd3dCommandList->ClearRenderTargetView(m_d3dRrvEnvirCubeMapCPUHandle[i], Colors::Black, 0, NULL);
		pd3dCommandList->ClearDepthStencilView(m_d3dDsvEnvirCubeMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvEnvirCubeMapCPUHandle[i], TRUE, &m_d3dDsvEnvirCubeMapCPUHandle);

		if (m_pd3dcbLights)
		{
			D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
			pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
		}

		if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, m_pCubeMapCamera[i]);
		if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, m_pCubeMapCamera[i]);

		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
				m_ppShaders[i]->Render(pd3dCommandList, m_pCubeMapCamera[i]);
		}
	}

	::TransitionResourceState(pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CBattleScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_SCENE_INFO) + 255) & ~255);

	m_pd3dcbSceneInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbSceneInfo->Map(0, NULL, (void **)&m_pcbMappedSceneInfo);
}

void CBattleScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT4 xmf4Random = XMFLOAT4((rand() + 20)* 0.7f, (rand() + 200) * 0.6f, (rand() + 2000) * 0.5f, (rand() + 20000) * 0.4f);

	m_pcbMappedSceneInfo->m_xmf4Random = xmf4Random;
	m_pcbMappedSceneInfo->m_fGravAcc = m_fGravAcc;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_SCENE_INFO, m_pd3dcbSceneInfo->GetGPUVirtualAddress());
}

void CBattleScene::ReleaseShaderVariables()
{
	if (m_pd3dcbSceneInfo)
	{
		m_pd3dcbSceneInfo->Unmap(0, NULL);
		m_pd3dcbSceneInfo->Release();

		m_pd3dcbSceneInfo = NULL;
	}

	CScene::ReleaseShaderVariables();
}

void CBattleScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pWireShader) m_pWireShader->OnPrepareRender(pd3dCommandList);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
	}
}

void CBattleScene::RenderEffects(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->Render(pd3dCommandList, pCamera);
}

void CBattleScene::RenderTestTexture(ID3D12GraphicsCommandList *pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUHandle)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, d3dSrvGPUHandle);

	m_pTestShader->Render(pd3dCommandList, NULL);
}

void CBattleScene::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CScene::PrepareRender(pd3dCommandList);

	if (m_pd3dShadowMap) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_SHADOW_MAP, m_d3dSrvShadowMapGPUHandle);
	if (m_pd3dEnvirCube) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE, m_d3dSrvEnvirCubeMapGPUHandle);

	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->PrepareRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->PrepareRender(pd3dCommandList);

	RenderShadowMap(pd3dCommandList);


	if (m_nFPSCount % 5 == 0)
	{
		RenderCubeMap(pd3dCommandList, m_pPlayer);
	}
}

void CBattleScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dOffScreenTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::TransitionResourceState(pd3dCommandList, m_pd3dGlowScreenTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::TransitionResourceState(pd3dCommandList, m_pd3dScreenNormalTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::TransitionResourceState(pd3dCommandList, m_pd3dMaskTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	pd3dCommandList->ClearRenderTargetView(m_d3dRrvOffScreenCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearRenderTargetView(m_d3dRrvGlowScreenCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearRenderTargetView(m_d3dRrvScreenNormalCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearRenderTargetView(m_d3dRrvMaskTextureCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearDepthStencilView(m_d3dDsvOffScreenCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	if (m_bSelfIllumination)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUDescHandle[4] = { m_d3dRrvOffScreenCPUHandle , m_d3dRrvGlowScreenCPUHandle, m_d3dRrvScreenNormalCPUHandle,
			m_d3dRrvMaskTextureCPUHandle };
		pd3dCommandList->OMSetRenderTargets(_countof(d3dCPUDescHandle), d3dCPUDescHandle, TRUE, &m_d3dDsvOffScreenCPUHandle);
	}
	else pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvOffScreenCPUHandle, TRUE, &m_d3dDsvOffScreenCPUHandle);

	if (pCamera) pCamera->OnPrepareRender(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
	}

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);

	if (m_pTerrain)
	{
		m_pTerrain->Render(pd3dCommandList, pCamera);
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}

	m_nFPSCount = (m_nFPSCount + 1) % 5;

	if (pCamera) m_xmf4x4CurrViewProjection = pCamera->GetViewProjMatrix();
}

void CBattleScene::PostProcessing(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_bMotionBlur || m_bSelfIllumination || m_bBloom)
		if (m_pd3dComputeRootSignature) pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);

	MotionBlur(pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	Bloom(pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	Blurring(pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	Combine(pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
}

void CBattleScene::MotionBlur(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	if (m_bMotionBlur)
	{
		float moveVel = Vector3::Length(Vector3::Subtract(m_pPlayer->GetPosition(), m_xmf3PrevPlayerPosition));

		moveVel *= FPS / m_fFPS;

		if (moveVel > 3.0f)
		{
			m_pComputeShader->SetMotionBlurPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 16, &m_xmf4x4PrevViewProjection, 0);

			XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(m_xmf4x4CurrViewProjection);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 16, &xmf4x4Inverse, 16);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &nWidth, 32);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &nHeight, 33);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &m_fFPS, 34);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_DEPTH, m_d3dSrvDepthStencilGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_MASK, m_d3dSrvMaskTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavMotionBlurScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, nHeight, 1);

			m_bMotionBlurred = true;
		}
	}
}

void CBattleScene::Bloom(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	UINT cxGroups = (UINT)ceilf(nWidth / 256.0f);
	UINT cyGroups = (UINT)ceilf(nHeight / 256.0f);

	if (m_bBloom)
	{
		m_pComputeShader->SetBrightFilterPipelineState(pd3dCommandList);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
		pd3dCommandList->Dispatch(nWidth, nHeight, 1);
	}

	if (m_bBloom || m_bSelfIllumination)
	{
		for (int i = 0; i < 1; i++)
		{
			m_pComputeShader->SetHorzPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvGlowScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavTempTextureGPUHandle);
			pd3dCommandList->Dispatch(cxGroups, nHeight, 1);

			m_pComputeShader->SetVertPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvTempTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, cyGroups, 1);
		}
	}
}

void CBattleScene::Blurring(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	UINT cxGroups = (UINT)ceilf(nWidth / 256.0f);
	UINT cyGroups = (UINT)ceilf(nHeight / 256.0f);

	if (m_bBloom || m_bSelfIllumination)
	{
		for (int i = 0; i < 1; i++)
		{
			m_pComputeShader->SetHorzPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvGlowScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavTempTextureGPUHandle);
			pd3dCommandList->Dispatch(cxGroups, nHeight, 1);

			m_pComputeShader->SetVertPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvTempTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, cyGroups, 1);
		}
	}
}

void CBattleScene::Combine(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	if (m_bBloom || m_bSelfIllumination || m_bMotionBlurred)
	{
		m_pComputeShader->Set2AddPipelineState(pd3dCommandList);

		if ((m_bBloom || m_bSelfIllumination) && m_bMotionBlurred)
		{
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvMotionBlurScreenGPUHandle);
		}
		else
		{
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
		}

		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B, m_d3dSrvGlowScreenGPUHandle);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavOffScreenGPUHandle);
		pd3dCommandList->Dispatch(nWidth, nHeight, 1);

		m_bMotionBlurred = false;
	}
}

void CBattleScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pUserInterface) m_pUserInterface->Render(pd3dCommandList, NULL);
	if (m_pMinimapShader) m_pMinimapShader->Render(pd3dCommandList, NULL);

	CScene::RenderUI(pd3dCommandList);
}

void CBattleScene::RenderOffScreen(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dOffScreenTexture) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, m_d3dSrvOffScreenGPUHandle);

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_SCREEN_EFFECT, 4, &m_xmf4ScreenColor, 0);

	if (m_bRenderEdge)
	{
		if (m_pd3dScreenNormalTexture) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_NORMAL_MAP, m_d3dSrvScreenNormalGPUHandle);
		m_pPostProcessingShader->RenderEdge(pd3dCommandList, NULL);
	}
	else
		m_pPostProcessingShader->Render(pd3dCommandList, NULL);
}

void CBattleScene::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dOffScreenTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::TransitionResourceState(pd3dCommandList, m_pd3dGlowScreenTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::TransitionResourceState(pd3dCommandList, m_pd3dScreenNormalTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::TransitionResourceState(pd3dCommandList, m_pd3dMaskTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->AfterRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->AfterRender(pd3dCommandList);

	if (pCamera) m_xmf4x4PrevViewProjection = pCamera->GetViewProjMatrix();
	if (m_pPlayer) m_xmf3PrevPlayerPosition = m_pPlayer->GetPosition();
}

void CBattleScene::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();

	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nEffectShaders; i++) if (m_ppEffectShaders[i]) m_ppEffectShaders[i]->ReleaseUploadBuffers();
	if (m_pParticleShader) m_pParticleShader->ReleaseUploadBuffers();
	if (m_pUserInterface) m_pUserInterface->ReleaseUploadBuffers();

	if (m_pGimGun) m_pGimGun->ReleaseUploadBuffers();
	if (m_pBazooka) m_pBazooka->ReleaseUploadBuffers();
	if (m_pMachineGun) m_pMachineGun->ReleaseUploadBuffers();
	if (m_pSaber) m_pSaber->ReleaseUploadBuffers();
	if (m_pMinimapShader) m_pMinimapShader->ReleaseUploadBuffers();
}

void CBattleScene::CheckCollision()
{
	std::vector<CGameObject*> vEnemys;

	for (int i = 0; i < SKINNED_OBJECT_GROUP; i++)
	{
		vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(i);

		for (const auto& Enemy : vEnemys)
		{
			if (!(Enemy->GetState() & OBJECT_STATE_SWORDING)) continue;
			CSword *pSword = (CSword*)(((CRobotObject*)Enemy)->GetWeapon(0));

			for (const auto& anotherE : vEnemys)
			{
				if (Enemy == anotherE) continue;
				if (anotherE->IsDie()) continue;
				if (!pSword->CollisionCheck(anotherE)) continue;
				if (Enemy->CheckDidHitObject(anotherE)) continue;

				XMFLOAT4 xmf4Color;
				if (pSword->GetType() & WEAPON_TYPE_OF_SABER) xmf4Color = XMFLOAT4(1.0f, 0.6f, 1.0f, 1.0f);
				else xmf4Color = XMFLOAT4(0.13f, 1.0f, 0.29f, 1.0f);

				AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);
				AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT_2, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);

				Enemy->AddHitObject(anotherE);

				gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
			}

			if (m_pPlayer)
			{
				if (m_pPlayer->IsDie()) continue;
				if (!pSword->CollisionCheck(m_pPlayer)) continue;
				if (Enemy->CheckDidHitObject(m_pPlayer)) continue;

				XMFLOAT4 xmf4Color;
				if (pSword->GetType() & WEAPON_TYPE_OF_SABER) xmf4Color = XMFLOAT4(1.0f, 0.6f, 1.0f, 1.0f);
				else xmf4Color = XMFLOAT4(0.13f, 1.0f, 0.29f, 1.0f);

				AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);
				AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT_2, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);

				Enemy->AddHitObject(m_pPlayer);

				gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
			}
		}

		if (m_pPlayer)
		{
			if (m_pPlayer->GetState() & OBJECT_STATE_SWORDING)
			{
				for (const auto& Enemy : vEnemys)
				{
					if (Enemy->IsDie()) continue;
					CSword *pSword = (CSword*)(m_pPlayer->GetWeapon(0));
					if (!pSword->CollisionCheck(Enemy)) continue;
					if (m_pPlayer->CheckDidHitObject(Enemy)) continue;

					pSword->SetOwnerTransform(m_pPlayer->GetRightHandFrame()->GetWorldTransf());
					pSword->UpdateWorldTransform();

					XMFLOAT4 xmf4Color;
					if (pSword->GetType() & WEAPON_TYPE_OF_SABER) xmf4Color = XMFLOAT4(1.0f, 0.6f, 1.0f, 1.0f);
					else xmf4Color = XMFLOAT4(0.13f, 1.0f, 0.29f, 1.0f);

					AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);
					AddSprite(SPRITE_EFFECT_INDEX_SWORD_HIT_2, pSword->GetBladePos(), EFFECT_ANIMATION_TYPE_ONE, xmf4Color);

					m_pPlayer->AddHitObject(Enemy);
					m_pPlayer->GetCamera()->SetShake(SK_ATTACK_SCALE);

					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
				}
			}
		}
	}

#ifndef ON_NETWORKING
	//std::vector<CGameObject*> vEnemys;
	std::vector<CGameObject*> vBullets;
	std::vector<CGameObject*> vBZKBullets;
	std::vector<CGameObject*> vMGBullets;
	std::vector<CGameObject*> vMeteors;

	//vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GUNDAM);
	vBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_GG_BULLET);
	vBZKBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_BZK_BULLET);
	vMGBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_MG_BULLET);
	vMeteors = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_METEOR);

	for (const auto& Enemy : vEnemys)
	{
		if (m_pPlayer->CollisionCheck(Enemy))
		{
			std::cout << "Collision Player By Enemy\n" << std::endl;
		}

		for (const auto& pBullet : vBullets)
		{
			if (!pBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), EFFECT_ANIMATION_TYPE_ONE, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					AddSprite(SPRITE_EFFECT_INDEX_GUN_HIT, pBullet->GetPosition(), EFFECT_ANIMATION_TYPE_ONE, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					pBullet->Delete();
					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pBullet->GetPosition());
				}
			}
		}

		for (const auto& pBZKBullet : vBZKBullets)
		{
			if (!pBZKBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBZKBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBZKBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), EFFECT_ANIMATION_TYPE_ONE, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					AddSprite(SPRITE_EFFECT_INDEX_EXPLOSION, pBZKBullet->GetPosition(), EFFECT_ANIMATION_TYPE_ONE, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					pBZKBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}

		for (const auto& pMGBullet : vMGBullets)
		{
			if (!pMGBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pMGBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pMGBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), EFFECT_ANIMATION_TYPE_ONE, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					AddSprite(SPRITE_EFFECT_INDEX_GUN_HIT, pMGBullet->GetPosition(), EFFECT_ANIMATION_TYPE_ONE, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					pMGBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pMGBullet->GetPosition());
				}
			}
		}

		for (const auto& pMeteor : vMeteors)
		{
			if (!pMeteor->IsDelete())
			{
				if (Enemy->CollisionCheck(pMeteor))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pMeteor->GetPosition(), XMFLOAT2(0.04f, 0.02f), EFFECT_ANIMATION_TYPE_ONE, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					AddSprite(SPRITE_EFFECT_INDEX_GUN_HIT, pMeteor->GetPosition(), EFFECT_ANIMATION_TYPE_ONE, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					pMeteor->Delete();

					std::cout << "Collision Enemy By Meteor \n" << std::endl;
				}
			}
		}
	}
#endif

	std::vector<CGameObject*> vObstacles;
	CObjectsShader* pObjectsShader = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS]);

	int nGroups = pObjectsShader->GetGroups();
	for (int i = 0; i < nGroups; i++)
	{
		vObstacles = pObjectsShader->GetObjects(i);

		for (const auto& Obstacle : vObstacles)
		{
			// 카메라 이동 O
			if (m_pPlayer->CollisionCheck(Obstacle))
			{
				if (m_nCurrentMap == SCENE_TYPE_COLONY) m_pPlayer->MoveToCollision(Obstacle);
				else m_pPlayer->MoveToCollisionByRadius(Obstacle);
			}
		}
	}
	m_pPlayer->MoveToWorldRange();

	FindAimToTargetDistance();
}

void CBattleScene::CheckCollisionPlayer()
{
	//std::vector<CGameObject*> *vObstacles;

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_HANGAR])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_DOUBLESQUARE])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_OCTAGON])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_OCTAGONLONGTIER])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SLOPETOP])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SQUARE])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STEEPLETOP])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_WALL])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}
}

void CBattleScene::AddParticle(int nType, XMFLOAT3 xmf3Position, int nNum, XMFLOAT4 xmf4Color)
{
	m_pParticleShader->AddParticle(nType, xmf3Position, nNum, xmf4Color);
};

void CBattleScene::AddSprite(int nEffect, XMFLOAT3 xmf3Position, int nType, XMFLOAT4 xmf4Color)
{
	float fSize = 0.0f;

	switch (nEffect)
	{
	case SPRITE_EFFECT_INDEX_SWORD_HIT:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_SWORD_HIT_SIZE;
		break;
	case SPRITE_EFFECT_INDEX_SWORD_HIT_2:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_SWORD_HIT_2_SIZE;
		break;
	case SPRITE_EFFECT_INDEX_GUN_HIT:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_GUN_HIT_SIZE;
		break;
	case SPRITE_EFFECT_INDEX_EXPLOSION:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_EXPLOSION_SIZE;
		break;
	case SPRITE_EFFECT_INDEX_DESTROY:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_DESTROY_SIZE;
		break;
	case SPRITE_EFFECT_INDEX_BEAM_HIT:
		fSize = (float)(rand() % 200) / 100.0f + SPRITE_EFFECT_BEAM_HIT_SIZE;
		break;
	}

	m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(nEffect, xmf3Position, XMFLOAT2(fSize, fSize), nType, rand() % 360, xmf4Color);
}

void CBattleScene::FindAimToTargetDistance()
{
	CWeapon *pWeapon = m_pPlayer->GetRHWeapon();
	int nType = pWeapon->GetType();
	if (!(nType & WEAPON_TYPE_OF_GUN)) return;

	float fDistance = 1000.0f;
	float fTemp = 0.0f;
	CGameObject *pTarget = NULL;
	XMFLOAT3 xmf3CameraPos = m_pPlayer->GetCamera()->GetPosition();
	XMVECTOR xmvCameraPos = XMLoadFloat3(&xmf3CameraPos);
	XMVECTOR xmvLook = XMLoadFloat3(&(m_pPlayer->GetCamera()->GetLookVector()));

	CGun *pGun = (CGun*)pWeapon;

	pGun->UpdateWorldTransform();
	XMFLOAT3 xmf3MuzzlePos = pGun->GetMuzzlePos();

	for (int i = 0; i < SKINNED_OBJECT_GROUP; i++)
	{
		std::vector<CGameObject*> vRobots = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GM + i);

		for (const auto& Robot : vRobots)
		{
			if (Robot->IsDie()) continue;

			// 카메라 이동 X 단 목표가 되지 않음.
			if (Robot->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
			{
				float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3MuzzlePos, xmf3CameraPos));

				if (fDistBetweenCnP < fTemp)
				{
					if (fDistance > fTemp)
					{
						fDistance = fTemp;
						pTarget = Robot;
					}
				}
			}
		}
	}

	if (m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS])
	{
		std::vector<CGameObject*> vObstacles;
		CObjectsShader* pObjectsShader = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS]);

		int nGroups = pObjectsShader->GetGroups();

		for (int i = 0; i < nGroups; i++)
		{
			vObstacles = pObjectsShader->GetObjects(i);

			for (const auto& Obstacle : vObstacles)
			{
				// 카메라 이동 O
				if (Obstacle->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
				{
					float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3MuzzlePos, xmf3CameraPos));

					if (fDistBetweenCnP < fTemp)
					{
						if (fDistance > fTemp)
						{
							fDistance = fTemp;
							pTarget = Obstacle;
						}
					}
				}
			}
		}
	}

	if (m_pTerrain)
	{
		if (m_pTerrain->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
		{
			float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3MuzzlePos, xmf3CameraPos));

			if (fDistBetweenCnP < fTemp)
			{
				if (fDistance > fTemp) fDistance = fTemp;
			}
		}
	}

	m_fCameraToTarget = fDistance;
}

//////////////////////////////// for Networking

void CBattleScene::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo)
{
	CGameObject* pGameObject = NULL;
	//pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

	if (m_pObjects[pCreateObjectInfo->Object_Index])
	{
		m_pObjects[pCreateObjectInfo->Object_Index]->Delete();
	}

	XMFLOAT3 xmf3Position = XMFLOAT3(pCreateObjectInfo->WorldMatrix._41, pCreateObjectInfo->WorldMatrix._42, pCreateObjectInfo->WorldMatrix._43);

	CObjectsShader *pObjectsShader = NULL;
	CEffectShader *pEffectShader = NULL;

	switch (pCreateObjectInfo->Object_Type)
	{
	case OBJECT_TYPE_PLAYER:
		pGameObject = new CRobotObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_SKINND_OBJECTS];

		m_ppShaders[INDEX_SHADER_SKINND_OBJECTS]->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, pCreateObjectInfo->Robot_Type, true, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS]);

		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetRightNozzleFrame());
		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetLeftNozzleFrame());

		m_ppEffectShaders[INDEX_SHADER_FOLLOW_SPRITE_EFFECTS]->SetFollowObject(FOLLOW_SPRITE_EFFECT_INDEX_BOOSTER, pGameObject, ((CRobotObject*)pGameObject)->GetRightNozzleFrame());
		m_ppEffectShaders[INDEX_SHADER_FOLLOW_SPRITE_EFFECTS]->SetFollowObject(FOLLOW_SPRITE_EFFECT_INDEX_BOOSTER, pGameObject, ((CRobotObject*)pGameObject)->GetLeftNozzleFrame());
		break;
	case OBJECT_TYPE_OBSTACLE:
		printf("Do not Apply Insert Obstacle\n");
		//((CObjectsShader*)m_ppShaders[INDEX_SHADER_OBSTACLE])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_ITEM_HEALING:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_REPAIR_ITEM, true, NULL);
		break;
	case OBJECT_TYPE_MACHINE_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundMGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_MG_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, rand() % 360, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case OBJECT_TYPE_BZK_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_BZK_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, rand() % 360, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case OBJECT_TYPE_BEAM_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_GG_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, rand() % 360, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case OBJECT_TYPE_ITEM_AMMO:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_AMMO_ITEM, true, NULL);
		break;
	case OBJECT_TYPE_METEOR:
		pGameObject = new Meteor();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_METEOR, true, NULL);
		break;
	}

	m_pObjects[pCreateObjectInfo->Object_Index] = pGameObject;
}

void CBattleScene::DeleteObject(int nIndex)
{
	if (m_pObjects[nIndex])
	{
		if (m_pObjects[nIndex]->GetType() & OBJECT_TYPE_ROBOT)
		{
			std::vector<CParticle*> vpParticles = ((CRobotObject*)m_pObjects[nIndex])->GetParticles();
			for (CParticle *pParticle : vpParticles) pParticle->Delete();
		}
		m_pObjects[nIndex]->Delete();
		m_pObjects[nIndex] = NULL;
	}
}

void CBattleScene::CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo)
{
	EFFECT_TYPE nEffectType = pCreateEffectInfo->efType;
	EFFECT_ANIMATION_TYPE nEffectAniType = pCreateEffectInfo->EftAnitType;
	CEffect *pEffect = NULL;
	CSprite *pSprite = NULL;
	float fSize = (float)(rand() % 200) / 100.0f + 10.0f;

	switch (nEffectType)
	{
	case EFFECT_TYPE::EFFECT_TYPE_HIT_FONT:
		m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pCreateEffectInfo->xmf3Position, XMFLOAT2(0.04f, 0.02f), 0, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_HIT:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

		AddSprite(SPRITE_EFFECT_INDEX_GUN_HIT, pCreateEffectInfo->xmf3Position, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_EXPLOSION:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);

		AddSprite(SPRITE_EFFECT_INDEX_EXPLOSION, pCreateEffectInfo->xmf3Position, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_GM_GUN:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
		m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS]->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, pCreateEffectInfo->xmf3Position, XMFLOAT2(1.0f, pCreateEffectInfo->fDistance), pCreateEffectInfo->xmf3Look, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_BEAM_RIFLE:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
		m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS]->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, pCreateEffectInfo->xmf3Position, XMFLOAT2(3.0f, pCreateEffectInfo->fDistance), pCreateEffectInfo->xmf3Look, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_BEAM_SNIPER:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
		m_ppEffectShaders[INDEX_SHADER_LASER_BEAM_EEFECTS]->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, pCreateEffectInfo->xmf3Position, XMFLOAT2(5.0f, pCreateEffectInfo->fDistance), pCreateEffectInfo->xmf3Look, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_BEAM_HIT:
		AddSprite(SPRITE_EFFECT_INDEX_BEAM_HIT, pCreateEffectInfo->xmf3Position, nEffectAniType, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		break;
	}
}

void CBattleScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	switch (pktID)
	{
	case PKT_ID_PLAYER_INFO:
	{
		PKT_PLAYER_INFO *pPacket = (PKT_PLAYER_INFO*)pktData;
		if (!m_pObjects[pPacket->ID]) break;

		m_pObjects[pPacket->ID]->SetWorldTransf(pPacket->WorldMatrix);
		m_pObjects[pPacket->ID]->SetPrepareRotate(0.0f, 180.0f, 0.0f);

		if (pPacket->isUpChangeAnimation)
		{
			CAnimationObject *pObject = (CAnimationObject*)m_pObjects[pPacket->ID];
			pObject->ChangeAnimation(ANIMATION_UP, 0, pPacket->Player_Up_Animation);
			pObject->SetAnimationTrackPosition(ANIMATION_UP, pPacket->UpAnimationPosition);
		}

		if (pPacket->isDownChangeAnimation)
		{
			CAnimationObject *pObject = (CAnimationObject*)m_pObjects[pPacket->ID];
			pObject->ChangeAnimation(ANIMATION_DOWN, 0, pPacket->Player_Down_Animation);
			pObject->SetAnimationTrackPosition(ANIMATION_DOWN, pPacket->DownAnimationPosition);
		}

		if (pPacket->isChangeWeapon)
		{
			CRobotObject *pObject = (CRobotObject*)m_pObjects[pPacket->ID];
			pObject->ChangeWeaponByType((WEAPON_TYPE)pPacket->Player_Weapon);
		}
		m_pObjects[pPacket->ID]->SetState(pPacket->State);
		break;
	}
	case PKT_ID_PLAYER_LIFE:
	{
		PKT_PLAYER_LIFE *pPacket = (PKT_PLAYER_LIFE*)pktData;

		if (!m_pObjects[pPacket->ID]) break;

		m_pObjects[pPacket->ID]->SetHitPoint(m_pObjects[pPacket->ID]->GetHitPoint() - pPacket->HP);
		break;
	}
	case PKT_ID_CREATE_OBJECT:
	{
		break;
	}
	case PKT_ID_DELETE_OBJECT:
	{
		PKT_DELETE_OBJECT *pPacket = (PKT_DELETE_OBJECT*)pktData;

		if (!m_pObjects[pPacket->Object_Index]) break;

		DeleteObject(pPacket->Object_Index);
		break;
	}
	case PKT_ID_TIME_INFO:
		break;
	case PKT_ID_UPDATE_OBJECT:
	{
		PKT_UPDATE_OBJECT *pPacket = (PKT_UPDATE_OBJECT*)pktData;

		if (!m_pObjects[pPacket->Object_Index]) break;

		XMFLOAT3 position = pPacket->Object_Position;
		m_pObjects[pPacket->Object_Index]->SetPosition(position);
		break;
	}
	case PKT_ID_CREATE_EFFECT:
		break;
	case PKT_ID_SCORE:
	{
		PKT_SCORE *pktScore = (PKT_SCORE*)pktData;
		wchar_t pstrText[16];

		wsprintfW(pstrText, L"%d", pktScore->RedScore);
		m_pFont->ChangeText(m_pRedScoreText, pstrText, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), RIGHT_ALIGN);

		wsprintfW(pstrText, L"%d", pktScore->BlueScore);
		m_pFont->ChangeText(m_pBlueScoreText, pstrText, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), LEFT_ALIGN);
		break;
	}
	case PKT_ID_PICK_ITEM:
	{
		PKT_PICK_ITEM *pPacket = (PKT_PICK_ITEM*)pktData;

		if (pPacket->Item_type == ITEM_TYPE_AMMO)
		{
			if (pPacket->ID == gClientIndex)
			{
				m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GM_GUN, pPacket->AMMO);
				m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, pPacket->AMMO);
				m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, pPacket->AMMO);
				gFmodSound.PlayFMODSound(gFmodSound.m_pSoundPickAmmo);
			}
		}
		else if (pPacket->Item_type == ITEM_TYPE_HEALING)
		{
			if (pPacket->ID == gClientIndex)
			{
				gFmodSound.PlayFMODSound(gFmodSound.m_pSoundPickHeal);
			}

			m_pPlayer->SetHitPoint(m_pPlayer->GetHitPoint() + pPacket->HP);
		}
		break;
	}
	case PKT_ID_PLAYER_DIE:
	{
		PKT_PLAYER_DIE *pPacket = (PKT_PLAYER_DIE*)pktData;

		XMFLOAT3 xmf3DestroyPos = XMFLOAT3(0.0f, 0.0f, 0.0f);

		if (gClientIndex == pPacket->id)
		{
			m_pPlayer->ProcessDie(pPacket->respawntime);
			xmf3DestroyPos = m_pPlayer->GetPosition();
		}
		else if (m_pObjects[pPacket->id])
		{
			m_pObjects[pPacket->id]->ProcessDie(pPacket->respawntime);
			xmf3DestroyPos = m_pObjects[pPacket->id]->GetPosition();
		}

		AddSprite(SPRITE_EFFECT_INDEX_DESTROY, Vector3::Add(xmf3DestroyPos, XMFLOAT3(0.0f, 15.0f, 0.0f)), EFFECT_ANIMATION_TYPE_ONE, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundDestroy);
		break;
	}
	case PKT_ID_PLAYER_RESPAWN:
	{
		PKT_PLAYER_RESPAWN *pPacket = (PKT_PLAYER_RESPAWN*)pktData;

		if (gClientIndex == pPacket->id)
		{
			m_pPlayer->CameraReset();

			m_pPlayer->ProcessRespawn(pPacket->hp, pPacket->point);

			if (pPacket->team == TEAM_TYPE_BLUE)
			{
				m_pPlayer->Rotate(0.0f, 180.0f, 0.0f);
			}
		}
		else
		{
			if (m_pObjects[pPacket->id])
			{

				m_pObjects[pPacket->id]->ProcessRespawn(pPacket->hp, pPacket->point);

				if (pPacket->team == TEAM_TYPE_BLUE)
				{
					m_pObjects[pPacket->id]->Rotate(0.0f, 180.0f, 0.0f);
				}
			}
		}
		break;
	}
	case PKT_ID_LOAD_COMPLETE_ALL:
	{
		m_pUserInterface->BattleNotifyStart();
		break;
	}
	case PKT_ID_GAME_END:
	{
		PKT_GAME_END *pPacket = (PKT_GAME_END*)pktData;

		bool bWin = CScene::m_nMyTeam == pPacket->WinTeam;

		m_pUserInterface->BattleNotifyEnd(bWin);
		m_bGameEnd = true;
		break;
	}
	case PKT_ID_KILL_MESSAGE:
	{
		PKT_KILL_MESSAGE *pPacket = (PKT_KILL_MESSAGE*)pktData;

		m_pUserInterface->AddKillInfo(pPacket->kill_name, static_cast<TEAM_TYPE>(pPacket->kill_team), pPacket->die_name, static_cast<TEAM_TYPE>(pPacket->die_team));
		break;
	}
	}
}

void CBattleScene::LeavePlayer(int nServerIndex)
{
	if (!m_pObjects[nServerIndex]) return;

	DeleteObject(nServerIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CColonyScene::CColonyScene() : CBattleScene()
{
	m_fGravAcc = GRAVITY;
}

CColonyScene::~CColonyScene()
{
}

void CColonyScene::BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CObstacleShader *pInstancingObstacleShader = new CObstacleShader();
	pInstancingObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pInstancingObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS] = pInstancingObstacleShader;
}

void CColonyScene::BuildTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 1.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("./Resource/Stage/HeightMap.raw"), 514, 514, 514, 514, xmf3Scale, xmf4Color);
}

void CColonyScene::BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CColonyScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	m_pLights->m_pLights[3].m_bEnable = false;
	m_pLights->m_pLights[3].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 500.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(500.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[4].m_bEnable = false;
	m_pLights->m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[4].m_fRange = 500.0f;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 500.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[5].m_bEnable = false;
	m_pLights->m_pLights[5].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[5].m_fRange = 500.0f;
	m_pLights->m_pLights[5].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(-500.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[5].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[5].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[6].m_bEnable = false;
	m_pLights->m_pLights[6].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[6].m_fRange = 500.0f;
	m_pLights->m_pLights[6].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, -500.0f);
	m_pLights->m_pLights[6].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[6].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

}

void CColonyScene::CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	XMFLOAT3 xmf3Look = XMFLOAT3(1.0f, -1.0f, 0.0f);

	XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 0.0f, -1.0f);

	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

	m_pLightCamera = new CLightCamera();

	m_pLightCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pLightCamera->GenerateOrthogonalMatrix(nWidth / 4.0f, nHeight / 4.0f, 0.0f, 5000.0f);
	m_pLightCamera->SetRight(xmf3Right);
	m_pLightCamera->SetUp(xmf3Up);
	m_pLightCamera->SetLook(xmf3Look);
	m_pLightCamera->SetPosition(XMFLOAT3(-1000.0f, 1300.0f, 0.0f));
	m_pLightCamera->SetViewPort(0.0f, 0.0f, float(nWidth), float(nHeight));
	m_pLightCamera->SetScissorRect(0, 0, nWidth, nHeight);

	m_pLightCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CColonyScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

	CBattleScene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CColonyScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));

	CBattleScene::UpdateShaderVariables(pd3dCommandList);
}

void CColonyScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
		m_pd3dcbLights = NULL;
	}

	CBattleScene::ReleaseShaderVariables();
}

void CColonyScene::RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	pd3dCommandList->ClearDepthStencilView(m_d3dDsvShadowMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	pd3dCommandList->OMSetRenderTargets(0, NULL, FALSE, &m_d3dDsvShadowMapCPUHandle);

	if (m_pLightCamera)
	{
		m_pLightCamera->GenerateViewMatrix();
		m_pLightCamera->OnPrepareRender(pd3dCommandList);
	}

	if (m_pTerrain)
		m_pTerrain->RenderToShadow(pd3dCommandList, m_pLightCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderToShadow(pd3dCommandList, m_pLightCamera);
	}

	if (m_pPlayer)
		m_pPlayer->RenderToShadow(pd3dCommandList, m_pLightCamera);

	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CColonyScene::StartScene(bool bBGMStop)
{
	if (bBGMStop) gFmodSound.StopFMODSound(gFmodSound.m_pBGMChannel);
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundColonyBGM, &(gFmodSound.m_pBGMChannel));
	//gFmodSound.m_pBGMChannel->setVolume(0.3f);
}

void CColonyScene::EndScene()
{
}

void CColonyScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CBattleScene::RenderUI(pd3dCommandList);

	//RenderTestTexture(pd3dCommandList, m_d3dSrvGlowScreenGPUHandle);
}

void CColonyScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	CBattleScene::ApplyRecvInfo(pktID, pktData);
	switch (pktID)
	{
	case PKT_ID_MAP_EVENT:
	{
		PKT_MAP_EVENT *pktScore = (PKT_MAP_EVENT*)pktData;

		switch (pktScore->type)
		{
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_ALERT:
			std::cout << "지상맵 이벤트 경고\n";
			Alert();
			break;
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_START:
			std::cout << "지상맵 이벤트 시작\n";
			m_fGravAcc = pktScore->gravity;
			break;
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_END:
			std::cout << "지상맵 이벤트 끝\n";
			m_fGravAcc = GRAVITY;
			break;
		}
		break;
	}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CSpaceScene::CSpaceScene() : CBattleScene()
{
	m_fGravAcc = 0.0f;
}

CSpaceScene::~CSpaceScene()
{
}

void CSpaceScene::BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CSpaceObstacleShader *pInstancingObstacleShader = new CSpaceObstacleShader();
	pInstancingObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pInstancingObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS] = pInstancingObstacleShader;
}

void CSpaceScene::BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CSpaceScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 500.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(500.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[4].m_bEnable = true;
	m_pLights->m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[4].m_fRange = 500.0f;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 500.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[5].m_bEnable = true;
	m_pLights->m_pLights[5].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[5].m_fRange = 500.0f;
	m_pLights->m_pLights[5].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(-500.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[5].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[5].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[6].m_bEnable = true;
	m_pLights->m_pLights[6].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[6].m_fRange = 500.0f;
	m_pLights->m_pLights[6].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[6].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, -500.0f);
	m_pLights->m_pLights[6].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[6].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

}

void CSpaceScene::CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	XMFLOAT3 xmf3Look = XMFLOAT3(1.0f, -1.0f, 0.0f);

	XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 0.0f, -1.0f);

	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

	m_pLightCamera = new CLightCamera();

	m_pLightCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pLightCamera->GenerateOrthogonalMatrix(nWidth * 0.5f, nHeight * 0.5f, 0.0f, 5000.0f);
	m_pLightCamera->SetRight(xmf3Right);
	m_pLightCamera->SetUp(xmf3Up);
	m_pLightCamera->SetLook(xmf3Look);
	m_pLightCamera->SetPosition(XMFLOAT3(-1000.0f, 1000.0f, 200.0f));
	m_pLightCamera->SetViewPort(0.0f, 0.0f, float(nWidth), float(nHeight));
	m_pLightCamera->SetScissorRect(0, 0, nWidth, nHeight);

	m_pLightCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSpaceScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

	CBattleScene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSpaceScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));

	CBattleScene::UpdateShaderVariables(pd3dCommandList);
}

void CSpaceScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
		m_pd3dcbLights = NULL;
	}

	CBattleScene::ReleaseShaderVariables();
}

void CSpaceScene::RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	pd3dCommandList->ClearDepthStencilView(m_d3dDsvShadowMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	pd3dCommandList->OMSetRenderTargets(0, NULL, FALSE, &m_d3dDsvShadowMapCPUHandle);

	if (m_pLightCamera)
	{
		m_pLightCamera->GenerateViewMatrix();
		m_pLightCamera->OnPrepareRender(pd3dCommandList);
	}

	if (m_pTerrain)
		m_pTerrain->RenderToShadow(pd3dCommandList, m_pLightCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderToShadow(pd3dCommandList, m_pLightCamera);
	}

	if (m_pPlayer)
		m_pPlayer->RenderToShadow(pd3dCommandList, m_pLightCamera);

	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CSpaceScene::StartScene(bool bBGMStop)
{
	if (bBGMStop) gFmodSound.StopFMODSound(gFmodSound.m_pBGMChannel);
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundSpaceBGM, &(gFmodSound.m_pBGMChannel));
	gFmodSound.m_pBGMChannel->setVolume(0.3f);
}

void CSpaceScene::EndScene()
{
}

void CSpaceScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CBattleScene::RenderUI(pd3dCommandList);

	//RenderTestTexture(pd3dCommandList, m_d3dSrvShadowMapGPUHandle);
}

void CSpaceScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	CBattleScene::ApplyRecvInfo(pktID, pktData);

	switch (pktID)
	{
	case PKT_ID_MAP_EVENT:
	{
		PKT_MAP_EVENT *pktScore = (PKT_MAP_EVENT*)pktData;

		switch (pktScore->type)
		{
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_ALERT:
			Alert();
			std::cout << "우주맵 이벤트 경고\n";
			break;
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_START:
			std::cout << "우주맵 이벤트 시작\n";
			m_fGravAcc = 0.0f;
			break;
		case MAP_EVENT_TYPE::MAP_EVENT_TYPE_END:
			std::cout << "우주맵 이벤트 끝\n";
			m_fGravAcc = 0.0f;
			break;
		}
		break;
	}
	}
}
