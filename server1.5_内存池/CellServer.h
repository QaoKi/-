#ifndef _CellServer_H
#define _CellServer_H

/*
	�������շ��ͽ��տͻ��˷��룬�������ڴ���ͻ��˵����ݣ�һ�����������ͻ���
*/

#ifdef _WIN32
#define FD_SETSIZE      2506
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
#include "ClientSocket.h"
#include "INetEvent.h"
#include "CellTask.h"
#include "CellSendMsg2ClientTask.h"
#include "CellTaskServer.h"
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>

using namespace std;

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SZIE
#endif 

typedef std::shared_ptr<CellSendMsg2ClientTask> CellSendMsg2ClientTaskPtr;

class CellServer;
typedef std::shared_ptr<CellServer> CellServerPtr;

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET);
	~CellServer();

public:
	//���ӿͻ��˵���Ϣ������
	void AddClientToBuff(ClientSocketPtr& pClient);

	//���ӵ���ʽ�ͻ�����
	void AddClient(ClientSocketPtr& pClient);
	ClientSocketPtr GetClient(SOCKET cSock);
	auto ClearClient(SOCKET cSock);

	void Start();

	int GetClientNum();

	bool Run();

	//�Ƿ�����
	bool isRun();

	void SetEventObj(INetEvent* event);

public:

	//��¼�յ���������Ϣ
	static int m_recvMsgCount;

	thread* m_pThread;

private:
	//�ر�����
	void Close();

	//�ر�ָ��socket
	void CloseSock(SOCKET sock);

	//��������
	int RecvData(SOCKET cSock);

	//��Ӧ����
	virtual void OnNetMsg(ClientSocketPtr pClient, DataHeader* header);

	void addSendTask(ClientSocketPtr pClient, DataHeader* header);

	//�����пͻ��˷�����
	//void SendDataToAll(DataHeader* data);

private:
	SOCKET m_sock;

	//�������
	CellTaskServer* m_TaskServer;

	//�¼�֪ͨ
	INetEvent* m_INetEvent;

	CELLTimestamp m_tTime;

	//���ջ�����
	char m_chRecvBuff[RECV_BUFF_SZIE];
	/*
	  ����Ͳ�ֻ��ſͻ��˵�socketֵ�����ǰ�ÿ���ͻ��˵�ClientSocket�ഫ��ȥ,
	  ע�⣬ClientSocket��������Ϊ�л��������ڴ�Ƚϴ����Դ��ָ��ȽϺ�
	*/
	//vector<SOCKET> m_clientArray;
	//��ʽ�ͻ�����
	map<SOCKET, ClientSocketPtr> m_clientArray;
	//��Ϊ������к����߳̽������ݽ����������¿ͻ�����ʱ�����̰߳��¿ͻ������������
	//�ٴӻ�����ȡ���¿ͻ��˷�����ʽ�ͻ�����
	vector<ClientSocketPtr> m_clientBuff;

	//�Ѿ��˳��˵Ŀͻ��˵�SOCKET
	set<SOCKET> m_clientExit;
	mutex m_clientBuffMutex;
};

#endif