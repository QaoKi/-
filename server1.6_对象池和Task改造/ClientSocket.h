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
#include "CELLObjectPool.hpp"

using namespace std;

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SZIE
#endif 

//�������������ʱʱ��
#define  CLIENT_HEART_DEAD_TIME 60000
//ָ����ʱ���ڣ��ѻ����������ݷ���ȥ
#define  CLIENT_SEND_BUFF_TIME 20

class ClientSocket;
typedef std::shared_ptr<ClientSocket> ClientSocketPtr;

class ClientSocket : public ObjectPoolBase<ClientSocket,1000>
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

	//��������,�����ͻ����������Ժ�
	int SendData(DataHeader* header);

	//�����ͻ������е����ݷ��͸��ͻ���
	int SendBuffReal();

	//���������յ�����ͻ��˷�����������֮�����ü�ʱʱ��
	void resetDTHeart();
	//�������
	bool checkHeart(time_t dt);

	//���÷�����Ϣʱ��
	void resetDTSendBuff();
	//���ʱ�䣬��ʱ������Ϣ
	bool checkSendBuff(time_t dt);

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
	//����������ʱ
	time_t m_dtHeart;
	//�ϴη�����Ϣ���ݵ�ʱ��
	time_t m_dtSendBuff;
};

#endif // !_Client_Socket_H_
