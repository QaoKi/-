#ifndef _ClientSocket_H
#define _ClientSocket_H

//Ϊÿ���ͻ��˴���һ���࣬ÿ�������Ӧһ���ͻ���

#include <stdio.h>
#include <iostream>
#include <memory>
#include <memory.h>
#include "CELL.h"
#include "MessageHeader.hpp"
#include "CELLObjectPool.hpp"
#include "CELLBuffer.h"
#include "CELLLog.h"

using namespace std;


//�������������ʱʱ��
#define  CLIENT_HEART_DEAD_TIME 60000
//ָ����ʱ���ڣ��ѻ����������ݷ���ȥ
#define  CLIENT_SEND_BUFF_TIME 200

class ClientSocket;
typedef std::shared_ptr<ClientSocket> ClientSocketPtr;

class ClientSocket : public ObjectPoolBase<ClientSocket,1000>
{
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET);
	virtual ~ClientSocket();

public:
	SOCKET GetSocket();

	void SetSocket(SOCKET sock);

	//�����ݷ��뷢�ͻ�����
	int SendData(netmsg_DataHeader* header);

	//�����ͻ������е����ݷ��͸��ͻ���
	int SendBuffReal();

	//��ΪClientSocket�ڲ������Ļ����������Զ����ⲿ�����ߣ�����Ҫ�ٷ�װһ������
	//�������ݵ����ջ��������Խ��ջ�������read4socket()���ٷ�װ
	int RecvData();

	//�Ƿ�����Ϣ���Դ����Խ��ջ�������hasMsg()���ٷ�װ
	bool hasMsg();

	//���ؽ��ջ������ĵ�һ��������Ϣ
	netmsg_DataHeader* front_msg();
	//�Ƴ��������еĵ�һ��������Ϣ
	void pop_front_msg();

	//���������յ�����ͻ��˷�����������֮�����ü�ʱʱ��
	void resetDTHeart();
	//�������
	bool checkHeart(time_t dt);

	//���ö�ʱ������Ϣ��ʱ��
	void resetDTSendBuff();
	//���ʱ�䣬��ʱ������Ϣ
	bool checkSendBuff(time_t dt);

private:

	//�ͻ����˳�
	void Close();

	SOCKET _ClientSocket;
	//������Ϣ������
	CELLBuffer _recvBuff = { RECV_BUFF_SZIE };

	//���ͻ�����,��ջ�Ϸ���ĳ�Ա�����������ֳ�ʼ����ʽ�����캯����ʼ���б��������
	CELLBuffer _sendBuff = { SEND_BUFF_SIZE };
	//����������ʱ
	time_t _dtHeart;
	//�ϴη�����Ϣ���ݵ�ʱ��
	time_t _dtSendBuff;

	//���ͻ���������д���������������ÿ�ΰѻ����������ݷ���ȥ�Ժ�����
	//�����ֵ����0��˵��֮ǰ��ҵ��ʹ���������ˣ���ô���ͻ��˿�д��ʱ��ֱ�ӷ���ȥ
	int _nSendBuffFullCount = 0;
};

#endif // !_Client_Socket_H_
