#include "pch.h"
#include "Framawork.h"

int main()
{
	Framawork framework;
	std::vector<std::thread> worker_threads;

	std::wcout.imbue(std::locale("korean"));

	framework.init();
	
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(std::thread{ framework.thread_process });
	std::thread accept_thread{ framework.accept_process };
	//thread timer_thread{ do_timer };

	//timer_thread.join();
	accept_thread.join();
	for (auto &th : worker_threads) th.join();

	return 0;
}