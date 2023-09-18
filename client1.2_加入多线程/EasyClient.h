#ifndef _EasyClient_H_
#define	_EasyClient_H_

/*
	��ϵͳsocket���ˣ�����һ�����ջ������ͷ��ͻ�������������Ͷ˷���̫�죬���ն�����������
	���ն˵Ľ��ջ����������ͻ�������ᵼ�»��������������޷��ٷ��ͣ���������

	Ҫ���������⣬�����ڽ��ն�����һ���ڴ棬��һ���ڶ����棬������ս��ջ��������Դﵽ��������������Ŀ��
	���緢�Ͷ�ÿ�η�������64�ֽڣ����ϵķ����ڽ��ն�����һ��4096�ֽڵĻ�������ÿ��ȡ���ݵ�ʱ������ݶ�ȡ��ȥ��
	ÿ�δ�socket��ȡ��4096�ֽڣ�Ҳ�ǲ��ϵ�ȡ�������Ͻ��ջ������ǲ��ᱻվ���ġ�����ÿ��ȡ�������ݣ�
	�����Ǵ���64�ֽڵģ��ü������ݣ���Ҳ������С��64�ֽڣ�����һ�����ݣ��������Ͳ�����ճ�����ٰ������
*/

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
#include "CELLThread.h"
#include "CELLMsgStream.h"
using namespace std;

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

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
	void Start();

	//�Ƿ�����
	bool isRun();

	//���������������
	int SendCmd(const char* chCmd);

	//�ر�����
	void Close();

private:

	//������ջ�����,ģ��ϵͳsocket�Ľ��ջ�����
	char m_chRecvBuff[RECV_BUFF_SZIE];
	//����ڶ�����������Ϣ������,ÿ�ΰѽ��ջ��������
	char m_chMsgBuff[RECV_BUFF_SZIE * 10];

	//��Ϣ������������β��λ��
	int m_nLastPos;

	CELLThread _thread;

private:

	//��������
	bool Run(CELLThread* pThread);

	//��������
	int RecvData();

	//��Ӧ����
	virtual void OnNetMsg(netmsg_DataHeader* header);

	//���������������
	int SendData(netmsg_DataHeader* header);

};

#endif // !_EasyClient_H_