#include "pch.h"
#include "Framework.h"

int main()
{
	setlocale(LC_ALL, "korean");

	Framework* framework = new Framework;
	if (framework->init())
	{
		if(framework->accpet_process())
			framework->recvn();
		else
			delete framework;
	}
	else
		delete framework;

	if(framework)
		delete framework;

	return 0;



}
