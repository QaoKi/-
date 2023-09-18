#include "ClientSocket.h"

ClientSocket::ClientSocket(SOCKET sock)
			:_sendBuff(SEND_BUFF_SIZE),_recvBuff(RECV_BUFF_SZIE)
{
	_ClientSocket = sock;
	resetDTHeart();
	resetDTSendBuff();
}

ClientSocket::~ClientSocket()
{
	Close();
}

void ClientSocket::SetSocket(SOCKET sock)
{
	_ClientSocket = sock;
}

int ClientSocket::SendData(netmsg_DataHeader* header)
{
	//��Ϊ��֪���ͻ����Ƿ���Խ��գ����Բ�����ֱ�ӵ���send��������
	//�Ȱ����ݽ��յ�����������������������ˣ����ش���

	//ԭ���ȷ��ͻ����������ٷ��Ͳ��������壬��Ϊֻ�п�д�¼������Ժ���ܷ���

	//Ҫ�������ݵĳ���
	int nSendLen = header->dataLength;
	//����ת��һ��
	const char* chData = (const char*)header;

	if (_sendBuff.push(chData, nSendLen))
	{
		return nSendLen;
	}
	return SOCKET_ERROR;
}

int ClientSocket::SendBuffReal()
{
	//��ʱ������Ϣ����ʱ����
	resetDTSendBuff();
	return _sendBuff.write2socket(_ClientSocket);
}

int ClientSocket::RecvData()
{
	return _recvBuff.read4socket(_ClientSocket);
}

bool ClientSocket::hasMsg()
{
	return _recvBuff.hasMag();
}

netmsg_DataHeader* ClientSocket::front_msg()
{
	return (netmsg_DataHeader*)_recvBuff.getData();
}

void ClientSocket::pop_front_msg()
{
	if (hasMsg())
		_recvBuff.pop(front_msg()->dataLength);
}

void ClientSocket::resetDTHeart()
{
	_dtHeart = 0;
}

bool ClientSocket::checkHeart(time_t dt)
{
	_dtHeart += dt;
	if (_dtHeart >= CLIENT_HEART_DEAD_TIME)
	{
		CELLLog::Info("checkHeart dead:c = %d,time=%d\n", _ClientSocket, _dtHeart);
		return true;
	}
	return false;
}

void ClientSocket::resetDTSendBuff()
{
	_dtSendBuff = 0;
}

bool ClientSocket::checkSendBuff(time_t dt)
{
	_dtSendBuff += dt;
	if (_dtSendBuff >= CLIENT_SEND_BUFF_TIME)
	{
		//���������ͻ������е����ݷ��ͳ�ȥ
		SendBuffReal();
		return true;
	}
	return false;
}

void ClientSocket::Close()
{
	if (_ClientSocket != INVALID_SOCKET)
	{

#ifdef _WIN32
		closesocket(_ClientSocket);
#else
		close(_ClientSocket);

#endif
		_ClientSocket = INVALID_SOCKET;
	}
}

SOCKET ClientSocket::GetSocket()
{
	return _ClientSocket;
}
