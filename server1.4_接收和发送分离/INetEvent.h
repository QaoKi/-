#ifndef _INetEvent_H
#define _INetEvent_H
#include "ClientSocket.h"

//����һ���¼��ĳ�����
class INetEvent
{
public:
	//���麯��
	//�ͻ����뿪�¼�
	virtual void ClientLeaveEvent(ClientSocket* pClient) = 0;
	//֮ǰ�õ��Ǿ�̬��������¼�յ��˶�������Ϣ���������¼�����ʽ
	virtual void RecvMsgEvent(ClientSocket* pClient) = 0;
};

#endif