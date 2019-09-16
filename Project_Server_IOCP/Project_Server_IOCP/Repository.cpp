#include "pch.h"
#include "Repository.h"


Repository::Repository()
{
}

Repository::~Repository()
{
	if (models.size())
	{
		for (auto& Model = models.begin(); Model != models.end();)
		{
			(*Model)->release();
			Model = models.erase(Model);
		}
	}
}

Model* Repository::get_model(char* file_name)
{
	for (const auto& Model : models)
	{
		if (!strcmp(Model->get_file_name(), file_name))
		{
			return Model;
		}
	}

	Model *pModel = Model::load_geometry_and_animation_from_file(file_name);
	pModel->add_ref();
	models.emplace_back(pModel);

	return pModel;
}