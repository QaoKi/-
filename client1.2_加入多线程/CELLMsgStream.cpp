#include "CELLMsgStream.h"


CELLRecvStream::CELLRecvStream(netmsg_DataHeader* header)
	:CELLStream((char*)header, header->dataLength)
{
	
}

CELLRecvStream::~CELLRecvStream()
{
}

CELLSendStream::CELLSendStream(netmsg_DataHeader* header)
	:CELLStream((char*)header, header->dataLength)
{

}

CELLSendStream::CELLSendStream(char* pData, int nSize, bool bDelete /*= false*/)
	: CELLStream(pData, nSize, bDelete)
{
	//Ԥ��ռ����Ϣ��������ռ�
	WriteInt32(0);
}

CELLSendStream::CELLSendStream(int nSize /*= 1024*/)
	: CELLStream(nSize)
{
	//Ԥ��ռ����Ϣ��������ռ�
	WriteInt32(0);
}

CELLSendStream::~CELLSendStream()
{

}

void CELLSendStream::finsh()
{
	//�õ���ǰ��ƫ��λ��
	int pos = getWritePos();
	//ƫ��λ����Ϊ0
	setWritePos(0);
	WriteInt32(pos);
	setWritePos(pos);
}
