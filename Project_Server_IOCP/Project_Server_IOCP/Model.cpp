#include "pch.h"
#include "Model.h"
#include "Mesh.h"

///////////////////////////////////////////////////////////////////////////////
Model::Model()
{
	world_matrix_ = Matrix4x4::Identity();
}

Model::~Model()
{
	if (meshes_)
	{
		delete meshes_;
		if (cube_meshes_) delete cube_meshes_;
	}
}

void Model::add_ref()
{
	references_++;

	if (sibling_) sibling_->add_ref();
	if (child_) child_->add_ref();
}

void Model::release()
{
	if (sibling_) sibling_->release();
	if (child_) child_->release();

	if (--references_ <= 0) delete this;
}

void Model::set_child(Model * child, bool add_reference)
{
	child->parent_ = this;
	if (add_reference) child->add_ref();

	if (child_)
	{
		child->sibling_ = child_->sibling_;
		child_->sibling_ = child;
	}
	else child_ = child;
}

void Model::set_mesh(Mesh *mesh, Cube_mesh *cube_mesh, bool Is_skinned)
{
	if (!meshes_)
	{
		num_meshes_++;
	}

	meshes_ = mesh;
	cube_meshes_ = cube_mesh;
}

void Model::get_num_meshes(int *meshes, int *skinned_meshes)
{
	(*meshes) += num_meshes_;

	if (sibling_) sibling_->get_num_meshes(meshes, skinned_meshes);
	if (child_) child_->get_num_meshes(meshes, skinned_meshes);
}

void Model::update_collision_box(std::vector<BoundingBox>& aabbs, int *index)
{
	if ((*index) == aabbs.size())
		return;

	if (meshes_) meshes_->aabbs_.Transform(aabbs[(*index)++], XMLoadFloat4x4(&world_matrix_));

	if (sibling_) sibling_->update_collision_box(aabbs, index);
	if (child_) child_->update_collision_box(aabbs, index);
}

void Model::update_world_transform(XMFLOAT4X4 *parent)
{
	world_matrix_ = (parent) ? Matrix4x4::Multiply(to_parent_matrix_, *parent) : to_parent_matrix_;

	if (sibling_) sibling_->update_world_transform(parent);
	if (child_) child_->update_world_transform(&world_matrix_);
}

Model* Model::load_geometry_and_animation_from_file(char *file_name)
{
	FILE *file;
	fopen_s(&file, file_name, "rb");
	printf("File : %s\n", file_name);

	std::string file_path = ::GetFilePath(file_name);
	printf("File Path : %s\n", file_path.c_str());

	Model *root_model = Model::load_model_from_file(file, file_name, file_path.c_str());
	int meshes = 0, skinned_meshes = 0;
	root_model->get_num_meshes(&meshes, &skinned_meshes);
	root_model->set_model_mesh_count(meshes, skinned_meshes);

	return root_model;
}

Model* Model::load_model_from_file(FILE *file, const char *file_name, const char *file_path)
{
	Model *model = NULL;

	while (true)
	{
		BYTE ste_len;
		char token[64] = { 0 };
		fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
		fread_s(token, sizeof(char) * 64, sizeof(char), ste_len, file);
		token[ste_len] = '\0';

		if (!strcmp(token, "<Frame>:"))
		{
			model = new Model();
			model->set_file_name(file_name);

			fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
			fread_s(model->model_name_, sizeof(char) * 64, sizeof(char), ste_len, file);
		}
		else if (!strcmp(token, "<TransformMatrix>:"))
		{
			fread_s(&model->to_parent_matrix_, sizeof(XMFLOAT4X4), sizeof(XMFLOAT4X4), 1, file);
		}
		else if (!strcmp(token, "<Mesh>:"))
		{
			Standard_mesh *pMesh = new Standard_mesh();
			pMesh->load_mesh_from_file(file);
			Cube_mesh *pCubeMesh = new Cube_mesh(pMesh->get_center(), pMesh->get_extents());
			model->set_mesh(pMesh, pCubeMesh, false);
		}
		else if (!strcmp(token, "<Materials>:"))
		{
			fread_s(&model->materials_, sizeof(int), sizeof(int), 1, file);

			if (model->materials_ > 0)
			{

				for (int i = 0; i < model->materials_; i++)
				{
					fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
					fread_s(token, sizeof(char) * 64, sizeof(char), ste_len, file);
					token[ste_len] = '\0';

					if (!strcmp(token, "<Material>:"))
					{
						XMFLOAT4 diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
						XMFLOAT4 emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4 specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4 ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4 reflection = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						float shininess = 0.0f;
						float transparency = 0.0f;

						float smoothness = 0.0f;
						float specular_highlight = 0.0f;
						float metallic = 0.0f;
						float glossy_reflection = 0.0f;

						while (true)
						{
							fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
							fread_s(token, sizeof(char) * 64, sizeof(char), ste_len, file);  // <Transforms>:
							token[ste_len] = '\0';

							if (!strcmp(token, "<AmbientColor>:"))
							{
								fread_s(&ambient, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, file);
							}
							else if (!strcmp(token, "<DiffuseColor>:"))
							{
								fread_s(&diffuse, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, file);
							}
							else if (!strcmp(token, "<EmissiveColor>:"))
							{
								fread_s(&emissive, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, file);
							}
							else if (!strcmp(token, "<ReflectionColor>:"))
							{
								fread_s(&reflection, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, file);
							}
							else if (!strcmp(token, "<SpecularColor>:"))
							{
								fread_s(&specular, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, file);
							}
							else if (!strcmp(token, "<ReflectionFactor>:"))
							{
								fread_s(&reflection.w, sizeof(float), sizeof(float), 1, file);
							}
							else if (!strcmp(token, "<Shininess>:"))
							{
								fread_s(&shininess, sizeof(float), sizeof(float), 1, file);
							}
							else if (!strcmp(token, "<SpecularFactor>:"))
							{
								fread_s(&specular.w, sizeof(float), sizeof(float), 1, file);
							}
							else if (!strcmp(token, "<Transparency>:"))
							{
								fread_s(&transparency, sizeof(float), sizeof(float), 1, file);
							}
							else if (!strcmp(token, "<DiffuseMap>:"))
							{
								char file_name[64] = { 0 };
								fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
								fread_s(file_name, sizeof(char) * 64, sizeof(char), ste_len, file);

								if (!strcmp(file_name, "null")) continue;

							}
							else if (!strcmp(token, "<NormalMap>:"))
							{
								char file_name[64] = { 0 };
								fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
								fread_s(file_name, sizeof(char) * 64, sizeof(char), ste_len, file);

								if (!strcmp(file_name, "null")) continue;

							}
							else if (!strcmp(token, "<SpecularFactorMap>:"))
							{
								char file_name[64] = { 0 };
								fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
								fread_s(file_name, sizeof(char) * 64, sizeof(char), ste_len, file);

								if (!strcmp(file_name, "null")) continue;

							}
							else if (!strcmp(token, "<SpecularMap>:"))
							{
								char file_name[64] = { 0 };
								fread_s(&ste_len, sizeof(BYTE), sizeof(BYTE), 1, file);
								fread_s(file_name, sizeof(char) * 64, sizeof(char), ste_len, file);

								if (!strcmp(file_name, "null")) continue;
							}
							else if (!strcmp(token, "</Material>"))
							{
								break;
							}
						}
					}
				}
			}
		}
		else if (!strcmp(token, "<Children>:"))
		{
			int num_children = 0;
			fread_s(&num_children, sizeof(int), sizeof(int), 1, file);

			if (num_children > 0)
			{
				for (int i = 0; i < num_children; i++)
				{
					Model *child = Model::load_model_from_file(file, file_name, file_path);
					if (child) model->set_child(child);
				}
			}

		}
		else if (!strcmp(token, "</Frame>"))
		{
			break;
		}
	}

	return model;
}
