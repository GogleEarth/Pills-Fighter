// pch.cpp: 미리 컴파일된 헤더에 해당하는 소스 파일입니다. 성공하려면 컴파일이 필요합니다.

#include "pch.h"


//
//DWORD WINAPI send_msg(LPVOID arg)
//{
//	int retval;
//	char buf(sizeof(PLAYER_INFO));
//	//float elapsedtime = 0.0f;
//	unsigned int i = 0;
//
//	while (true)
//	{
//		//auto start = std::chrono::high_resolution_clock::now();
//		if (i >= 5)
//		{
//			m.lock();
//			if (msg_queue.size() != 0)
//			{
//				PLAYER_INFO p_info = msg_queue.front();
//				int id = p_info.client_id;
//				memcpy(&buf, &p_info, sizeof(PLAYER_INFO));
//				for (SOCKET d : clients)
//				{
//					retval = send(d, &buf, sizeof(PLAYER_INFO), 0);
//					if (retval == SOCKET_ERROR)
//					{
//						err_display("send");
//						break;
//					}
//					std::cout << id << "의 정보 " << retval << "바이트 보냈음\n";
//				}
//				msg_queue.pop();
//			}
//			m.unlock();
//			i = 0;
//		}
//		//auto end = std::chrono::high_resolution_clock::now();
//		//auto du = end - start;
//		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(du).count() / 1000.0f << "지남\n";
//		//elapsedtime += std::chrono::duration_cast<std::chrono::milliseconds>(du).count() / 1000.0f;
//		i++;
//	}
//
//	return 0;
//}
// 일반적으로 이 파일을 무시하지만 미리 컴파일된 헤더를 사용하는 경우 유지합니다.
