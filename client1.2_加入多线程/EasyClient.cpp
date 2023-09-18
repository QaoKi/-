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
		printf("<socket=%d>close old socket...\n", (int)m_sock);
		Close();
	}
	m_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == m_sock)
	{
#ifdef _WIN32
		printf("error,create socket fail...error code��%d\n", GetLastError());
#else
		perror("create socket fail:\n");
#endif
	}
	else {
		//printf("����socket=<%d>�ɹ�...\n", (int)m_sock);
	}

	//�˿ڸ���
	int opt = 1;
	setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
}

int EasyClient::Connect(const char* ip, unsigned short port)
{
	if (!isRun())
	{
		InitSocket();
	}
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
		printf("<socket=%d>error,connect server <%s:%d>fail,error code:%d...\n", m_sock, ip, port, GetLastError());
#else
		perror("connect server fail:\n");
#endif
	}
	else {
		printf("<socket=%d>connect server<%s:%d> succ...\n",m_sock, ip, port);
	}
	return ret;
}

void EasyClient::Start()
{
	_thread.Start(
		//onCreate
		nullptr,
		//onRun
		[this](CELLThread* pThread) {
		Run(pThread);
	},
		//onClose
		nullptr
		);
}

bool EasyClient::Run(CELLThread* pThread)
{
	//�һ��selectģ��
	fd_set fdRead;

	while (pThread->isRun())
	{
		FD_ZERO(&fdRead);
		FD_SET(m_sock, &fdRead);
		timeval tTimeOut = { 0,0 };

		int nReady = select(m_sock + 1, &fdRead, NULL, NULL, &tTimeOut);

		if (nReady == 0){
			continue;
		}
		else if (nReady < 0)
		{
			printf("<socket=%d>select exit\n", m_sock);
			pThread->Exit();
			break;
		}
		else
		{
			if (FD_ISSET(m_sock, &fdRead))
			{
				FD_CLR(m_sock, &fdRead);
				RecvData();
			}
		}
	}
	return true;
}

void EasyClient::Close()
{
	_thread.Close();
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
		printf("<socket=%d>Disconnect from the server,exit\n", m_sock);
		Close();
		return -1;
	}
	//����ȡ�������ݿ�������Ϣ��������β��,���յ����ٿ�����ȥ����
	memcpy(m_chMsgBuff + m_nLastPos, m_chRecvBuff, nLen);
	//��Ϣ������������β��λ�ú���
	m_nLastPos += nLen;

	//ֻҪ��Ϣ�������е����ݣ����ȴ�����Ϣͷnetmsg_DataHeader����
	while (m_nLastPos >= sizeof(netmsg_DataHeader))
	{
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		netmsg_DataHeader* header = (netmsg_DataHeader*)m_chMsgBuff;
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

void EasyClient::OnNetMsg(netmsg_DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		struct LoginResult* loginResult = (LoginResult*)header;
		printf("recv server request:CMD_LOGIN_RESULT,data length:%d,data:%s\n",loginResult->dataLength, loginResult->data);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		struct LogoutResult* logoutResult = (LogoutResult*)header;
		printf("recv server request:CMD_LOGOUT_RESULT,data length:%d\n", logoutResult->dataLength);
	}
	case CMD_NEW_USER_JOIN:
	{
		struct NewUserJoin* newUser = (NewUserJoin*)header;
		printf("recv server request:CMD_NEW_USER_JOIN,new use socket:%d,data length:%d\n", newUser->scok,newUser->dataLength);
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

int EasyClient::SendData(netmsg_DataHeader* header)
{
	if (isRun() && header)
	{
		return send(m_sock, (const char*)header, header->dataLength, 0);
	}
	return SOCKET_ERROR;

}

