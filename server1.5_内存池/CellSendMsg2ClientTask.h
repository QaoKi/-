#ifndef _CellSendMsg2ClientTask_H
#define _CellSendMsg2ClientTask_H
#include "CellTask.h"
#include "ClientSocket.h"
#include "MessageHeader.hpp"

//�̳�������࣬����������񣺷�����Ϣ���ͻ��˵�����
class CellSendMsg2ClientTask : public CellTask
{
public:
	CellSendMsg2ClientTask(ClientSocketPtr& pClient, DataHeader* pHeader);
	~CellSendMsg2ClientTask();

	void doTask();

private:
	ClientSocketPtr m_pClient;
	DataHeader* m_pHeader;
};

#endif 

