#ifndef _CELL_MSG_STREAM_H_
#define _CELL_MSG_STREAM_H_
#include "MessageHeader.hpp"
#include "CELLStream.h"

//��Ϣ�����ֽ���
class CELLRecvStream : public CELLStream
{
public:
	//���յ�ʱ����Ϣ��ʽ��������netmsg_DataHeader����ʽ
	CELLRecvStream(netmsg_DataHeader* header);

	uint16_t getNetMsgCmd();
};


class CELLSendStream : public CELLStream
{
public:
	CELLSendStream(char* pData, int nSize, bool bDelete = false);
	CELLSendStream(int nSize = 1024);

	void setNetMsgCmd(uint16_t cmd);

	//Ҫ���͵���Ϣд���Ժ����һ��
	void finsh();
};

#endif // _CELL_MSG_STREAM_H_
