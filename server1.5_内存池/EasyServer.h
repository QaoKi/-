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
#include "ClientSocket.h"
#include "INetEvent.h"
#include "CellServer.h"
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>

using namespace std;

/*
	1.1:Ϊÿ���ͻ��˷���һ����Ϣ�����������пͻ��˹���һ�����ջ�����
*/
/*
	1.2:���߳��������տͻ������ӺͿͻ��˶Ͽ������¼������̣߳�CellServer�����ڴ�����������
		���̺߳����߳�֮��ͨ��������н���ͨ��
*/
/*
	1.3:ͨ��ʵ�鷢�֣�recv�Ľ���������Զ����send�ķ������������ԣ�Ҫ�Է������ݽ��иĽ�
		ͨ����һ����ʱ�䷢�ͣ���������һ���������ٷ��͵ķ�ʽ���Ľ����ݵķ���

	1.4:���⿪�̴߳������˷����ͻ��˵����ݣ�CellTaskServer�����п��̷߳������ݣ�CellTaskServer����CellServer�������
		�ڴ棬���У��ж��ٸ�CellServer�����շ��񣩣����ж��ٸ�CellTaskServer�����ͷ���

	1.5���ڴ����
		�����ڴ���Ƭ�Ĳ�����ʹ�������ȶ�����Ч������
		�ڴ�أ���ϵͳ�������㹻��С���ڴ棬�ɳ����Լ�����
		����أ������㹻��Ķ��󣬼��ٴ����ͷŶ��������
		windowsϵͳ�е��ļ�������4k����ģ���ʹ��Сû��4k��Ҳ��ռ��4k�Ĵ�С����ռ�õĴ�С������4k��������
		�Լ���Ƶ��ڴ�أ�һ����Ϊ5���أ�
			1~64�ֽ�Ϊһ���أ�65~128�ֽ�Ϊһ���أ�129~256�ֽ�Ϊһ���أ�257~512�ֽ�Ϊһ���أ�513~1024�ֽ�Ϊһ����
			���������ڴ���64�ֽ����ڣ��ͷ���һ��1~64�ֽڵĳؿռ䣬�����������ڴ���65~128�ֽڡ�����
*/

#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240 * 5
#define SEND_BUFF_SIZE RECV_BUFF_SZIE
#endif 

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
	map<SOCKET, ClientSocketPtr> m_clientArray;
	//����CellServer
	vector<CellServerPtr> m_CellServerArray;

public:
	void AddClient(ClientSocketPtr& pClient);
	ClientSocketPtr GetClient(SOCKET cSock);
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
	virtual void ClientLeaveEvent(ClientSocketPtr& pClient);

	virtual void RecvMsgEvent(ClientSocketPtr& pClient);

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
	void AddClientToCellServer(ClientSocketPtr& pClient);

	//���һ�����յ���������Ϣ
	void Time4Msg();

	//�ر�����
	//void Close();

	//�ر�ָ��socket
	void CloseSock(SOCKET sock);
};

#endif

