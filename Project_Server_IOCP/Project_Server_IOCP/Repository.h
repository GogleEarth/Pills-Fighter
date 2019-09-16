#pragma once

#include"Model.h"


class Repository
{
	std::vector<Model*> models;
public:
	Repository();
	~Repository();

	Model* get_model(char *file_name);
};


