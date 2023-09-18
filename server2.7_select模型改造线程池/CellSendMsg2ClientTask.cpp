#include "CellSendMsg2ClientTask.h"



CellSendMsg2ClientTask::CellSendMsg2ClientTask(ClientSocketPtr& pClient, netmsg_DataHeader* pHeader)
{
	_pClient = pClient;
	_pHeader = pHeader;
}


CellSendMsg2ClientTask::~CellSendMsg2ClientTask()
{
}

void CellSendMsg2ClientTask::doTask()
{
	_pClient->SendData(_pHeader);
	//ִ���������Ժ󣬰�����ɾ�������ǿͻ��˲���ɾ��
	if (_pHeader)
	{
		delete _pHeader;
		_pHeader = NULL;
	}
}
