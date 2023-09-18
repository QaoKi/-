#ifndef _EasyClient_H_
#define	_EasyClient_H_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <iostream>
#include "MessageHeader.hpp"
#include <vector>
using namespace std;
class EasyClient
{
public:
	EasyClient();
	virtual ~EasyClient();

public:
	SOCKET m_sock;

public:
	//��ʼ��socket
	void InitSocket();

	//���ӷ�����
	int Connect(const char* ip, unsigned short port);

	//��ʼ����
	bool Run();

	//�Ƿ�����
	bool isRun();

	//���������������
	int SendCmd(const char* chCmd);

	//�ر�����
	void Close();

private:

	//��������
	int RecvData();

	//��Ӧ����
	virtual void OnNetMsg(DataHeader* header);

	//���������������
	int SendData(DataHeader* header);

};

#endif // !_EasyClient_H_