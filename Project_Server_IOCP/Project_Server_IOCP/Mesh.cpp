#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	if (positions_) delete[] positions_;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Cube_mesh::Cube_mesh(XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents) : Mesh()
{
	vertices_ = 8;

	float fx = xmf3Extents.x, fy = xmf3Extents.y, fz = xmf3Extents.z;

	positions_ = new XMFLOAT3[vertices_];

	positions_[0] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	positions_[1] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	positions_[2] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	positions_[3] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	positions_[4] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	positions_[5] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	positions_[6] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);
	positions_[7] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);
}

Cube_mesh::~Cube_mesh()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FindXYZ(XMFLOAT3* pPositions, UINT nVertices, XMFLOAT3& Center, XMFLOAT3& Extents)
{
	XMFLOAT3 MinMaxVertex[2]; // [0] Min, [1] Max
	MinMaxVertex[0] = MinMaxVertex[1] = pPositions[0];

	for (UINT i = 1; i < nVertices; i++)
	{
		XMFLOAT3 Position = pPositions[i];

		if (Position.x < MinMaxVertex[0].x) MinMaxVertex[0].x = Position.x;
		if (Position.y < MinMaxVertex[0].y) MinMaxVertex[0].y = Position.y;
		if (Position.z < MinMaxVertex[0].z) MinMaxVertex[0].z = Position.z;

		if (Position.x > MinMaxVertex[1].x) MinMaxVertex[1].x = Position.x;
		if (Position.y > MinMaxVertex[1].y) MinMaxVertex[1].y = Position.y;
		if (Position.z > MinMaxVertex[1].z) MinMaxVertex[1].z = Position.z;
	}

	Center = Vector3::Add(MinMaxVertex[0], MinMaxVertex[1]);
	Center.x /= 2; Center.y /= 2; Center.z /= 2;

	Extents = Vector3::Subtract(MinMaxVertex[1], MinMaxVertex[0]);
	Extents.x /= 2; Extents.y /= 2; Extents.z /= 2;
}

Standard_mesh::Standard_mesh() : Mesh()
{
}

Standard_mesh::~Standard_mesh()
{
}

void Standard_mesh::load_mesh_from_file(FILE *pfile)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	type_ |= 0x01;

	fread_s(&vertices_, sizeof(UINT), sizeof(int), 1, pfile);

	fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
	fread_s(name_, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

	while (true)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<Positions>:"))
		{
			positions_ = new XMFLOAT3[vertices_];
			fread_s(positions_, sizeof(XMFLOAT3) * vertices_, sizeof(XMFLOAT3), vertices_, pfile);
		}
		else if (!strcmp(pstrToken, "<TextureCoords>:"))
		{
			XMFLOAT2* texture_coords0 = new XMFLOAT2[vertices_];
			fread_s(texture_coords0, sizeof(XMFLOAT2) * vertices_, sizeof(XMFLOAT2), vertices_, pfile);
			delete texture_coords0;
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			XMFLOAT3* normals = new XMFLOAT3[vertices_];
			fread_s(normals, sizeof(XMFLOAT3) * vertices_, sizeof(XMFLOAT3), vertices_, pfile);
			delete normals;
		}
		else if (!strcmp(pstrToken, "<Binormals>:"))
		{
			XMFLOAT3* binormals = new XMFLOAT3[vertices_];
			fread_s(binormals, sizeof(XMFLOAT3) * vertices_, sizeof(XMFLOAT3), vertices_, pfile);
			delete binormals;
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			XMFLOAT3* tangents = new XMFLOAT3[vertices_];
			fread_s(tangents, sizeof(XMFLOAT3) * vertices_, sizeof(XMFLOAT3), vertices_, pfile);
			delete tangents;
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			fread_s(&submeshes_, sizeof(int), sizeof(int), 1, pfile);

			subsetindices_ = new int[submeshes_];
			subsetindices_array_ = new UINT*[submeshes_];

			if (submeshes_ > 0)
			{
				for (int i = 0; i < submeshes_; i++)
				{
					fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
					fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
					pstrToken[nstrLength] = '\0';

					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						fread_s(&subsetindices_[i], sizeof(int), sizeof(int), 1, pfile);

						if (subsetindices_[i] > 0)
						{
							subsetindices_array_[i] = new UINT[subsetindices_[i]];

							fread_s(subsetindices_array_[i], sizeof(int) * subsetindices_[i], sizeof(int), subsetindices_[i], pfile);
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}

	XMFLOAT3 xmf3Center;
	XMFLOAT3 xmf3Extents;
	FindXYZ(positions_, vertices_, xmf3Center, xmf3Extents);
	set_aabb(xmf3Center, xmf3Extents);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
