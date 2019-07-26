#ifndef _CellServer_H
#define _CellServer_H
#include "CELLLog.h"
/*
	�������շ��ͽ��տͻ��˷��룬�������ڴ���ͻ��˵����ݣ�һ�����������ͻ���
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>
#include <functional>

#include "CELL.h"
#include "MessageHeader.hpp"
#include "CELLTimestamp.h"
#include "ClientSocket.h"
#include "INetEvent.h"
//#include "CellTask.h"
//#include "CellSendMsg2ClientTask.h"
#include "CellTaskServer.h"
#include "CELLSemaphore.h"
#include "CELLEpollEvent.h"

using namespace std;

//typedef std::shared_ptr<CellSendMsg2ClientTask> CellSendMsg2ClientTaskPtr;

class CellServer;
typedef std::shared_ptr<CellServer> CellServerPtr;
class CellServer
{
public:
	CellServer(int id);
	~CellServer();

public:
	//���ӿͻ��˵���Ϣ������
	void AddClientToBuff(ClientSocketPtr& pClient);

	//���ӵ���ʽ�ͻ�����
	void AddClient(ClientSocketPtr& pClient);
	ClientSocketPtr GetClient(SOCKET cSock);
	auto ClearClient(SOCKET cSock);

	void Start();
	//�ر�
	void Close();

	int GetClientNum();

	bool Run(CELLThread * pThread);

	void SetEventObj(INetEvent* event);

public:

	//��¼�յ���������Ϣ
	static int _recvMsgCount;

	thread* _pThread;

private:
	//���
	int _id;
	//�߳�
	CELLThread _thread;

	//������пͻ���
	void ClearAllClient();

	//��������
	int RecvData(int sock,int events);

	//��������
	int SendData(int sock, int events);

	//��Ӧ����
	virtual void OnNetMsg(ClientSocketPtr& pClient, netmsg_DataHeader* header);

	void addSendTask(ClientSocketPtr pClient, netmsg_DataHeader* header);

	//�����пͻ��˷�����
	//void SendDataToAll(DataHeader* data);

	//���������Ŀͻ��˵�������ʱʱ��
	void checkHeart();
private:

	//�������
	CellTaskServerPtr _TaskServerPtr;

	//�¼�֪ͨ
	INetEvent* _INetEvent;

	CELLTimestamp _tTime;
	/*
	  ����Ͳ�ֻ��ſͻ��˵�socketֵ�����ǰ�ÿ���ͻ��˵�ClientSocket�ഫ��ȥ,
	  ע�⣬ClientSocket��������Ϊ�л��������ڴ�Ƚϴ����Դ��ָ��ȽϺ�
	*/
	//vector<SOCKET> _clientArray;
	//��ʽ�ͻ�����
	//��ʽ�ͻ����жӲ���Ҫ������Ϊ������̶߳�ռ�ģ������������߳̽��н���
	map<SOCKET, ClientSocketPtr> _clientArray;

	//��Ϊ������к����߳̽������ݽ����������¿ͻ�����ʱ�����̰߳��¿ͻ������������
	//�ٴӻ�����ȡ���¿ͻ��˷�����ʽ�ͻ�����
	vector<ClientSocketPtr> _clientBuff;
	//����֮������Ҫ��������Ϊ_clientBuff�������߳̽����ģ����߳���ʱ���ܻ�����µĿͻ��˽����������̻߳���ʱȡ��������Ҫ����
	mutex _clientBuffMutex;
	//�Ѿ��˳��˵Ŀͻ��˵�SOCKET
	set<SOCKET> _clientExit;
	
	//��Run��ʼִ�е�ʱ��ʱ���ʼ��һ��
	//��û�пͻ��˵�ʱ��ÿ����Ҫ����һ��ʱ��
	time_t _oldTime;

	CELLEpollEvent _myevent = { FD_SETSIZE };
	EventCallBack _RecvCallBack;
	EventCallBack _SendCallBack;
};

#endif