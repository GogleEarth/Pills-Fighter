#include "pch.h"
#include "Framawork.h"

int main()
{
	std::wcout.imbue(std::locale("korean"));

	Framawork* framework = new Framawork();
	std::vector<std::thread> worker_threads;

	framework->init();
	
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(std::thread{ [&]() { framework->thread_process(); } });
	std::thread accept_thread{ [&]() { framework->accept_process(); } };
	std::thread timer_thread{ [&]() { framework->timer_process(); } };

	timer_thread.join();
	accept_thread.join();
	for (auto &th : worker_threads) th.join();

	return 0;
}