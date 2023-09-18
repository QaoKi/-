#include "ClientSocket.h"

ClientSocket::ClientSocket(SOCKET sock)
{
	m_ClientSocket = sock;
	m_nMsgLastPos = 0;
	m_nSendLastPos = 0;
	resetDTHeart();
	resetDTSendBuff();
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

			//������Ϣ���ͼ�ʱ
			resetDTSendBuff();

			if (nRet == SOCKET_ERROR)
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

int ClientSocket::SendBuffReal()
{
	//������������
	int nRet = 0;
	if (m_nSendLastPos > 0)
	{
		nRet = send(m_ClientSocket, m_chSendBuf, m_nSendLastPos, 0);
		m_nSendLastPos = 0;
		//������Ϣ���ͼ�ʱ
		resetDTSendBuff();
	}
	return nRet;
}

void ClientSocket::resetDTHeart()
{
	m_dtHeart = 0;
}

bool ClientSocket::checkHeart(time_t dt)
{
	m_dtHeart += dt;
	if (m_dtHeart >= CLIENT_HEART_DEAD_TIME)
	{
		printf("checkHeart dead:c = %d,time=%d\n", m_ClientSocket, m_dtHeart);
		return true;
	}
	return false;
}

void ClientSocket::resetDTSendBuff()
{
	m_dtSendBuff = 0;
}

bool ClientSocket::checkSendBuff(time_t dt)
{
	m_dtSendBuff += dt;
	if (m_dtSendBuff >= CLIENT_SEND_BUFF_TIME)
	{
		printf("checkSendBuff c = %d,time=%d\n,length:%d", m_ClientSocket, m_dtSendBuff,m_nSendLastPos);
		//���������ͻ������е����ݷ��ͳ�ȥ
		SendBuffReal();
		return true;
	}
	return false;
}

SOCKET ClientSocket::GetSocket()
{
	return m_ClientSocket;
}
