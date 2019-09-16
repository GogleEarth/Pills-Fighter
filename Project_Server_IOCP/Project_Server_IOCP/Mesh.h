#pragma once

class Mesh
{
protected:
	UINT vertices_ = 0;

	XMFLOAT3* positions_ = NULL;

	int submeshes_ = 0;
	int* subsetindices_ = NULL;
	UINT** subsetindices_array_ = NULL;

	char name_[64] = { 0 };
	int type_ = 0x00;

public:
	Mesh();
	virtual ~Mesh();

	BoundingBox aabbs_;
	void set_aabb(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents) { aabbs_ = BoundingBox(xmCenter, xmExtents); }
	void set_aabb(BoundingBox xmAABB) { aabbs_ = xmAABB; }
	XMFLOAT3& get_extents() { return aabbs_.Extents; }
	XMFLOAT3& get_center() { return aabbs_.Center; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Cube_mesh : public Mesh
{
public:
	Cube_mesh(XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents);
	~Cube_mesh();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Standard_mesh : public Mesh
{
public:
	Standard_mesh();
	~Standard_mesh();

	void load_mesh_from_file(FILE *file);
};
