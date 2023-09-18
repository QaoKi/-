#ifndef _EasySever_H
#define _EasySever_H

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <iostream>
#include "MessageHeader.hpp"
#include <vector>
#include <set>

using namespace std;


class EasyServer
{
public:
	EasyServer();
	virtual ~EasyServer();

public:
	SOCKET m_sock;
	vector<SOCKET> m_clientFd;

public:
	void AddClient(SOCKET cSock);
	void ClearClient(SOCKET cSock);

public:
	//��ʼ��socket
	void InitSocket();

	//��ip�Ͷ˿�
	int Bind(const char* ip, unsigned short port);

	//�����˿ں�
	int Listen(int n);

	//��ʼ����
	bool Run();

	//�Ƿ�����
	bool isRun();

private:
	//���ܿͻ�������
	SOCKET Accept();

	//��������
	int RecvData(SOCKET cSock);

	//��Ӧ����
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header);

	//��ָ���ͻ��˷�������
	int SendData(SOCKET cSock, DataHeader* header);

	//�����пͻ��˷�����
	void SendDataToAll(DataHeader* data);

	//�ر�����
	void Close();
private:
	//�ر�ָ��socket
	void CloseSock(SOCKET sock);
};

#endif

