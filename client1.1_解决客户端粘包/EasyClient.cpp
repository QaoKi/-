#include "EasyClient.h"



EasyClient::EasyClient()
{
	m_sock = INVALID_SOCKET;
	memset(m_chRecvBuff, 0, sizeof(m_chRecvBuff));
	memset(m_chMsgBuff, 0, sizeof(m_chMsgBuff));
	m_nLastPos = 0;
}


EasyClient::~EasyClient()
{
	Close();
}

void EasyClient::InitSocket()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	if (INVALID_SOCKET != m_sock)
	{
		printf("<socket=%d>�رվ�����...\n", (int)m_sock);
		Close();
	}
	m_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == m_sock)
	{
#ifdef _WIN32
		printf("���󣬽���socketʧ��...����%d\n", GetLastError());
#else
		perror("����socketʧ��:");
#endif
	}
	else {
		printf("����socket=<%d>�ɹ�...\n", (int)m_sock);
	}

	//�˿ڸ���
	int opt = 1;
	setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
}

int EasyClient::Connect(const char* ip, unsigned short port)
{
	struct sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
#ifdef _WIN32
	serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	serverAddr.sin_addr.s_addr = inet_addr(ip);
#endif

	int ret = connect(m_sock, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (ret < 0)
	{
#ifdef _WIN32
		printf("<socket=%d>�������ӷ�����<%s:%d>ʧ��,�����룺%d...\n", m_sock, ip, port, GetLastError());
#else
		perror("���ӷ�����ʧ��:");
#endif
	}
	else {
		printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�...\n",m_sock, ip, port);
	}
	return ret;
}

bool EasyClient::Run()
{
	//�һ��selectģ��
	fd_set fdRead;

	//while (isRun())
	//{
		FD_ZERO(&fdRead);
		FD_SET(m_sock, &fdRead);
		timeval tTimeOut = { 0,0 };

		int nReady = select(m_sock + 1, &fdRead, NULL, NULL, &tTimeOut);

		if (nReady == 0){
			return false;
		}
		else if (nReady < 0)
		{
			printf("<socket=%d>select�������\n", m_sock);
			Close();
			return false;
		}
		else
		{
			if (FD_ISSET(m_sock, &fdRead))
			{
				FD_CLR(m_sock, &fdRead);
				RecvData();
			}
		}
	//}
	return true;
}

void EasyClient::Close()
{
	if (m_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(m_sock);
		//���Windows socket����
		WSACleanup();
#else
		close(m_sock);
#endif // _WIN32
		m_sock = INVALID_SOCKET;
	}
}

bool EasyClient::isRun()
{
	return m_sock != INVALID_SOCKET;
}

int EasyClient::RecvData()
{
	//�Ƚ��յ����ջ�������
	int nLen = recv(m_sock, m_chRecvBuff, sizeof(m_chRecvBuff), 0);
	if (nLen <= 0)
	{
		printf("<socket=%d>��������Ͽ����ӣ����������\n", m_sock);
		Close();
		return -1;
	}
	//����ȡ�������ݿ�������Ϣ��������β��,���յ����ٿ�����ȥ����
	memcpy(m_chMsgBuff + m_nLastPos, m_chRecvBuff, nLen);
	//��Ϣ������������β��λ�ú���
	m_nLastPos += nLen;

	//ֻҪ��Ϣ�������е����ݣ����ȴ�����ϢͷDataHeader����
	while (m_nLastPos >= sizeof(DataHeader))
	{
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		DataHeader* header = (DataHeader*)m_chMsgBuff;
		//��Ϣ�������е����ݳ��ȣ����ڵ��ڵ�ǰ���ݵĳ���
		if (m_nLastPos >= header->dataLength)
		{
			//��Ϊ����memcpy֮��header�����ݾͱ��ˣ���Ӧ��header->dataLengthҲ�ͱ��ˣ����ƶ�nLastPos��ʱ������
			//�����ȼ�¼����
			//��Ϣ����δ�������Ϣ����
			int size = m_nLastPos - header->dataLength;
			//������Ϣ
			OnNetMsg(header);
			//����Ϣ��������ȥ��������Ϣ����������Ϣ֮��������Ƶ�������ͷ����
			memcpy(m_chMsgBuff, m_chMsgBuff + header->dataLength, size);
			//�ƶ���Ϣ������β��λ��
			m_nLastPos = size;
		}
		else
		{
			//��Ϣ������ʣ�����ݲ���һ��������Ϣ
			break;
		}
	}

	return 0;
}

void EasyClient::OnNetMsg(DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		struct LoginResult* loginResult = (LoginResult*)header;
		printf("�յ�����˻ظ���CMD_LOGIN_RESULT,���ݳ��ȣ�%d�����ݣ�%s\n",loginResult->dataLength, loginResult->data);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		struct LogoutResult* logoutResult = (LogoutResult*)header;
		printf("�յ�����˻ظ���CMD_LOGOUT_RESULT,���ݳ��ȣ�%d\n", logoutResult->dataLength);
	}
	case CMD_NEW_USER_JOIN:
	{
		struct NewUserJoin* newUser = (NewUserJoin*)header;
		printf("�յ�����˻ظ���CMD_NEW_USER_JOIN,���û�socket:%d,���ݳ��ȣ�%d\n", newUser->scok,newUser->dataLength);
	}
	break;
	default:
		break;
	}

}


int EasyClient::SendCmd(const char* chCmd)
{
	string strCmd(chCmd);

	if (strCmd.compare("login") == 0){
		Login login;
		strcpy(login.userName, "zf");
		strcpy(login.PassWord, "123");
		SendData(&login);
	}
	else if (strCmd.compare("logout") == 0) {
		Logout logout;
		strcpy(logout.userName, "zf");
		SendData(&logout);
	}
	else if(strCmd.compare("exit") == 0) {
		ExitConnect exit;
		SendData(&exit);
	}
	else
	{
		return -1;
	}
	return 0;
}

int EasyClient::SendData(DataHeader* header)
{
	if (isRun() && header)
	{
		return send(m_sock, (const char*)header, header->dataLength, 0);
	}
	return SOCKET_ERROR;

}

