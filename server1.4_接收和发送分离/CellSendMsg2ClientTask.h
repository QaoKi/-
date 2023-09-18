#ifndef _CellSendMsg2ClientTask_H
#define _CellSendMsg2ClientTask_H
#include "CellTask.h"
#include "ClientSocket.h"
#include "MessageHeader.hpp"

//�̳�������࣬����������񣺷�����Ϣ���ͻ��˵�����
class CellSendMsg2ClientTask : public CellTask
{
public:
	CellSendMsg2ClientTask(ClientSocket* pClient, DataHeader* pHeader);
	~CellSendMsg2ClientTask();

	void doTask();

private:
	ClientSocket* m_pClient;
	DataHeader* m_pHeader;
};

#endif 

