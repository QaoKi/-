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
	int nRet = SOCKET_ERROR;
	if (!header)
		return nRet;

	//�����ͻ����������ٷ���

	//Ҫ�������ݵĳ���
	int nSendLen = header->dataLength;
	//����ת��һ��
	const char* chData = (const char*)header;

	while (true)
	{
		//���Ҫ���͵����ݳ��ȣ����ڷ��ͻ���������ĳ���
		if (nSendLen > SEND_BUFF_SIZE - _nSendLastPos)
		{
			//������Է��͵ĳ���
			int nCopyLen = SEND_BUFF_SIZE - _nSendLastPos;
			memcpy(_chSendBuf + _nSendLastPos, chData, nCopyLen);
			//ʣ��Ҫ���͵����ݳ��Ⱥ�����
			nSendLen -= nCopyLen;
			chData += nCopyLen;

			nRet = send(_ClientSocket, _chSendBuf, sizeof(_chSendBuf), 0);
			_nSendLastPos = 0;

			//������Ϣ���ͼ�ʱ
			resetDTSendBuff();

			if (nRet == SOCKET_ERROR)
				return nRet;
		}
		else
		{
			memcpy(_chSendBuf + _nSendLastPos, chData, nSendLen);
			_nSendLastPos += nSendLen;
			break;
		}
	}
	return 0;
}

int ClientSocket::SendBuffReal()
{
	//������������
	int nRet = 0;
	if (_nSendLastPos > 0)
	{
		nRet = send(_ClientSocket, _chSendBuf, _nSendLastPos, 0);
		_nSendLastPos = 0;
		//������Ϣ���ͼ�ʱ
		resetDTSendBuff();
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
