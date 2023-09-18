#ifndef _EasySever_H
#define _EasySever_H

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
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <atomic>

using namespace std;

/*
	1.1:Ϊÿ���ͻ��˷���һ����Ϣ�����������пͻ��˹���һ�����ջ�����
*/
/*
	1.2:���߳��������տͻ������ӺͿͻ��˶Ͽ������¼������߳����ڴ�����������
		���̺߳����߳�֮��ͨ��������н���ͨ��
*/


#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE



//Ϊÿ���ͻ��˴���һ������
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

private:

	SOCKET m_ClientSocket;
	//��Ϣ������
	char m_chMsgBuf[RECV_BUFF_SZIE * 5];
	//��Ϣ������������β��λ��
	int m_nLastPos;
};

class INetEvent
{
public:
	//���麯��
	//�ͻ����뿪�¼�
	virtual void ClientLeaveEvent(ClientSocket* pClient) = 0;
	//֮ǰ�õ��Ǿ�̬��������¼�յ��˶�������Ϣ���������¼�����ʽ
	virtual void RecvMsgEvent(ClientSocket* pClient) = 0;
};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET);
	~CellServer();

public:
	//���ӿͻ��˵���Ϣ������
	void AddClientToBuff(ClientSocket* pClient);
	
	//���ӵ���ʽ�ͻ�����
	void AddClient(ClientSocket* pClient);
	ClientSocket* GetClient(SOCKET cSock);
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
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header);

	//��������
	int SendData(SOCKET cSock, DataHeader* header);

	//�����пͻ��˷�����
	void SendDataToAll(DataHeader* data);

private:
	SOCKET m_sock;

	//�¼�֪ͨ
	INetEvent* m_INetEvent;

	CELLTimestamp m_tTime;

	//���ջ�����
	char m_chRecvBuff[RECV_BUFF_SZIE * 5];
	/*
	  ����Ͳ�ֻ��ſͻ��˵�socketֵ�����ǰ�ÿ���ͻ��˵�ClientSocket�ഫ��ȥ,
	  ע�⣬ClientSocket��������Ϊ�л��������ڴ�Ƚϴ����Դ��ָ��ȽϺ�
	*/
	//vector<SOCKET> m_clientArray;
	//��ʽ�ͻ�����
	map<SOCKET, ClientSocket*> m_clientArray;
	//��Ϊ������к����߳̽������ݽ����������¿ͻ�����ʱ�����̰߳��¿ͻ������������
	//�ٴӻ�����ȡ���¿ͻ��˷�����ʽ�ͻ�����
	vector<ClientSocket*> m_clientBuff;

	//�Ѿ��˳��˵Ŀͻ��˵�SOCKET
	set<SOCKET> m_clientExit;
	mutex m_clientBuffMutex;

};

class EasyServer : public INetEvent
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
	//vector<SOCKET> m_clientArray;
	map<SOCKET,ClientSocket*> m_clientArray;
	//����CellServer
	vector<CellServer*> m_CellServerArray;

public:
	void AddClient(ClientSocket* pClient);
	ClientSocket* GetClient(SOCKET cSock);
	void ClearClient(SOCKET cSock);
public:
	//��ʼ��socket
	void InitSocket();

	//��ip�Ͷ˿�
	int Bind(const char* ip, unsigned short port);

	//�����˿ں�
	int Listen(int n);

	//����������Ϣ���߳�,����ʼ�����ͻ�������
	void Start(int nCellServerCount);

	//��ʼ����
	bool Run();

	//�Ƿ�����
	bool isRun();

	//�ͻ����뿪�¼�
	virtual void ClientLeaveEvent(ClientSocket* pClient);

	virtual void RecvMsgEvent(ClientSocket* pClient);

private:
	//���ܻ�����
	//char m_chRecvBuff[RECV_BUFF_SZIE];
	CELLTimestamp m_tTime;

	//��¼�ͻ��˷�������������Ϣ
	//��Ϊ�ж���̻߳�ͬʱ������������Ҫ��ԭ������
	atomic_int nRecvMsgCoutn;

private:
	//���ܿͻ�������
	SOCKET Accept();

	//�ѿͻ��˷���CelleServer
	void AddClientToCellServer(ClientSocket* pClient);

	//���һ�����յ���������Ϣ
	void Time4Msg();

	//�ر�����
	//void Close();

	//�ر�ָ��socket
	void CloseSock(SOCKET sock);
};

#endif

