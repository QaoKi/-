#include "ClientSocket.h"

ClientSocket::ClientSocket(SOCKET sock)
{
	_ClientSocket = sock;
	_nMsgLastPos = 0;
	_nSendLastPos = 0;
	resetDTHeart();
	resetDTSendBuff();
	memset(_chMsgBuf, 0, sizeof(_chMsgBuf));
	memset(_chSendBuf, 0, sizeof(_chSendBuf));
}

ClientSocket::~ClientSocket()
{
	Close();
}

int ClientSocket::GetLastPos()
{
	return _nMsgLastPos;
}

void ClientSocket::SetLastPos(int pos)
{
	_nMsgLastPos = pos;
}
void ClientSocket::SetSocket(SOCKET sock)
{
	_ClientSocket = sock;
}

char* ClientSocket::GetMsgBuf()
{
	return _chMsgBuf;
}

int ClientSocket::SendData(netmsg_DataHeader* header)
{
	//��Ϊ��֪���ͻ����Ƿ���Խ��գ����Բ�����ֱ�ӵ���send��������
	//�Ȱ����ݽ��յ�����������������������ˣ����ش���
	int nRet = SOCKET_ERROR;

	//ԭ���ȷ��ͻ����������ٷ��Ͳ��������壬��Ϊֻ�п�д�¼������Ժ���ܷ���

	//Ҫ�������ݵĳ���
	int nSendLen = header->dataLength;
	//����ת��һ��
	const char* chData = (const char*)header;

	//��Ҫ���͵����ݳ��ȣ�С�ڵ��ڷ��ͻ���������ĳ��ȣ������ݷŵ�������
	if (nSendLen <= SEND_BUFF_SIZE - _nSendLastPos)
	{
		memcpy(_chSendBuf + _nSendLastPos, chData, nSendLen);
		_nSendLastPos += nSendLen;
		nRet = nSendLen;
		if (_nSendLastPos == SEND_BUFF_SIZE)
		{
			_nSendBuffFullCount++;
		}
	}
	else
	{
		_nSendBuffFullCount++;
	}
	return nRet;
}

int ClientSocket::SendBuffReal()
{
	int nRet = SOCKET_ERROR;
	//������������
	//��_nSendLastPos == 0 ˵����������û�����ݣ�������󣬷���0

	if (_nSendLastPos == 0)
	{
		nRet = 0;
	}
	else if (_nSendLastPos > 0 && INVALID_SOCKET != _ClientSocket)
	{
		nRet = send(_ClientSocket, _chSendBuf, _nSendLastPos, 0);
		
		if (nRet > 0)
		{
			//���û�����ָ��λ��
			_nSendLastPos = 0;
			//���û�����������
			_nSendBuffFullCount = 0;
			//������Ϣ���ͼ�ʱ
			resetDTSendBuff();
		}
		else
		{
			//��Ϊsend()�ķ���ֵ����Ϊ0��send()����0��ʾ��������Ҫ����һ��
			nRet = SOCKET_ERROR;
		}
	}
	return nRet;
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
		printf("checkHeart dead:c = %d,time=%d\n", _ClientSocket, _dtHeart);
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
		printf("checkSendBuff c = %d,time=%d\n,length:%d\n", _ClientSocket, _dtSendBuff,_nSendLastPos);
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
