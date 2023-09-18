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
#include "CELLTimestamp.h"
#include <vector>
#include <set>
#include <map>

using namespace std;

/*
	Ϊÿ���ͻ��˷���һ����Ϣ�����������пͻ��˹���һ�����ջ�����
*/

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

//Ϊÿ���ͻ��˴���һ������
class ClientSocket
{
public:
	ClientSocket(SOCKET m_ClientSocket);
	virtual ~ClientSocket();

public:
	SOCKET GetSocket();

	int GetLastPos();

	void SetLastPos(int pos);

	char* GetMsgBuf();

private:

	SOCKET m_ClientSocket;
	//��Ϣ������
	char m_chMsgBuf[RECV_BUFF_SZIE * 10];
	//��Ϣ������������β��λ��
	int m_nLastPos;
};


class EasyServer
{
public:
	EasyServer();
	virtual ~EasyServer();

public:
	SOCKET m_sock;
	/*
	  ����Ͳ�ֻ��ſͻ��˵�socketֵ�����ǰ�ÿ���ͻ��˵�ClientSocket�ഫ��ȥ,
	  ע�⣬ClientSocket��������Ϊ�л��������ڴ�Ƚϴ����Դ��ָ��ȽϺ�
	*/
	//vector<SOCKET> m_clientFd;
	map<SOCKET,ClientSocket*> m_clientFd;

public:
	void AddClient(SOCKET cSock);
	ClientSocket* GetClient(SOCKET cSock);
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
	//���ܻ�����
	char m_chRecvBuff[RECV_BUFF_SZIE];
	CELLTimestamp m_tTime;
	//���������յ���������Ϣ
	int m_recvCount;

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

	//�ر�ָ��socket
	void CloseSock(SOCKET sock);
};

#endif

