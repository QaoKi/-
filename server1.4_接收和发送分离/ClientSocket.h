#ifndef _ClientSocket_H
#define _ClientSocket_H

//Ϊÿ���ͻ��˴���һ���࣬ÿ�������Ӧһ���ͻ���

#ifdef _WIN32
#include<windows.h>
#else
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <iostream>
#include "MessageHeader.hpp"

using namespace std;

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240 * 5
#define SEND_BUFF_SIZE RECV_BUFF_SZIE
#endif 

class ClientSocket
{
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET);
	virtual ~ClientSocket();

public:
	SOCKET GetSocket();

	int GetLastPos();

	void SetLastPos(int pos);

	void SetSocket(SOCKET sock);

	char* GetMsgBuf();

	//��������
	int SendData(DataHeader* header);

private:

	SOCKET m_ClientSocket;
	//��Ϣ������
	char m_chMsgBuf[RECV_BUFF_SZIE];
	//��Ϣ������������β��λ��
	int m_nMsgLastPos;

	//���ͻ������������ͻ����������ٷ���
	char m_chSendBuf[SEND_BUFF_SIZE];
	//���ͻ�����������β��λ��
	int m_nSendLastPos;
};

#endif // !_Client_Socket_H_
