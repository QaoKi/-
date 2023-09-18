#include "ClientSocket.h"

ClientSocket::ClientSocket(SOCKET sock)
{
	m_ClientSocket = sock;
	m_nMsgLastPos = 0;
	m_nSendLastPos = 0;
	memset(m_chMsgBuf, 0, sizeof(m_chMsgBuf));
	memset(m_chSendBuf, 0, sizeof(m_chSendBuf));
}

ClientSocket::~ClientSocket()
{
	m_ClientSocket = INVALID_SOCKET;
	m_nMsgLastPos = 0;
	memset(m_chMsgBuf, 0, sizeof(m_chMsgBuf));
}

int ClientSocket::GetLastPos()
{
	return m_nMsgLastPos;
}

void ClientSocket::SetLastPos(int pos)
{
	m_nMsgLastPos = pos;
}
void ClientSocket::SetSocket(SOCKET sock)
{
	m_ClientSocket = sock;
}

char* ClientSocket::GetMsgBuf()
{
	return m_chMsgBuf;
}

int ClientSocket::SendData(DataHeader* header)
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
		if (nSendLen > SEND_BUFF_SIZE - m_nSendLastPos)
		{
			//������Է��͵ĳ���
			int nCopyLen = SEND_BUFF_SIZE - m_nSendLastPos;
			memcpy(m_chSendBuf + m_nSendLastPos, chData, nCopyLen);
			//ʣ��Ҫ���͵����ݳ��Ⱥ�����
			nSendLen -= nCopyLen;
			chData += nCopyLen;

			nRet = send(m_ClientSocket, m_chSendBuf, sizeof(m_chSendBuf), 0);
			m_nSendLastPos = 0;

			if (nRet = SOCKET_ERROR)
				return nRet;
		}
		else
		{
			memcpy(m_chSendBuf + m_nSendLastPos, chData, nSendLen);
			m_nSendLastPos += nSendLen;
			break;
		}
	}
	return 0;
}

SOCKET ClientSocket::GetSocket()
{
	return m_ClientSocket;
}
