#include "CELLNetWork.h"



CELLNetWork::CELLNetWork()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

#ifndef _WIN32
	//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
	signal(SIGPIPE, SIG_IGN);
#endif

}


CELLNetWork::~CELLNetWork()
{
#ifdef _WIN32
	//���Windows socket����
	WSACleanup();
#endif
}

void CELLNetWork::Init()
{
	static CELLNetWork obj;
}
