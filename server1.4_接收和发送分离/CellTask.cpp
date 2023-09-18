#include "CellTask.h"



CellTask::CellTask()
{
}


CellTask::~CellTask()
{
}

void CellTask::doTask()
{

}

CellSendMsg2ClientTask::CellSendMsg2ClientTask(ClientSocket* pClient, DataHeader* pHeader)
{
	m_pClient = pClient;
	m_pHeader = pHeader;
}


CellSendMsg2ClientTask::~CellSendMsg2ClientTask()
{
}

void CellSendMsg2ClientTask::doTask()
{
	m_pClient->SendData(m_pHeader);
	//ִ���������Ժ󣬰�����ɾ�������ǿͻ��˲���ɾ��
	if (m_pHeader)
	{
		delete m_pHeader;
		m_pHeader = NULL;
	}
}

