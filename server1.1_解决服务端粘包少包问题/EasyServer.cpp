#include "EasyServer.h"

ClientSocket::ClientSocket(SOCKET m_ClientSocket)
{
	m_ClientSocket = INVALID_SOCKET;
	memset(m_chMsgBuf, 0, sizeof(m_chMsgBuf));
}

int ClientSocket::GetLastPos()
{
	return m_nLastPos;
}

void ClientSocket::SetLastPos(int pos)
{
	m_nLastPos = pos;
}

char* ClientSocket::GetMsgBuf()
{
	return m_chMsgBuf;
}

ClientSocket::~ClientSocket()
{
	
}

SOCKET ClientSocket::GetSocket()
{
	return m_ClientSocket;
}


EasyServer::EasyServer()
{
	m_sock = INVALID_SOCKET;
	m_recvCount = 0;
	memset(m_chRecvBuff, 0, sizeof(m_chRecvBuff));
}


EasyServer::~EasyServer()
{
	Close();
}

void EasyServer::AddClient(SOCKET cSock)
{
	if (m_clientFd.find(cSock) == m_clientFd.end())
	{
		ClientSocket* client = new ClientSocket(cSock);
		if (client)
			m_clientFd[cSock] = client;
	}
}

ClientSocket* EasyServer::GetClient(SOCKET cSock)
{
	auto it = m_clientFd.find(cSock);
	if (it != m_clientFd.end())
	{
		return it->second;
	}

	return NULL;
}

void EasyServer::ClearClient(SOCKET cSock)
{
	auto it = m_clientFd.find(cSock);
	if (it != m_clientFd.end())
	{
		if (it->second)
			delete it->second;
		m_clientFd.erase(it);
	}
}

void EasyServer::InitSocket()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	if (INVALID_SOCKET != m_sock)
	{
		printf("<socket=%d>�رվ�����...\n",(int)m_sock);
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
int EasyServer::Bind(const char* ip, unsigned short port)
{
	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

#ifdef _WIN32
	if (ip){
		sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else {
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
	}
#else
	if (ip) {
		sin.sin_addr.s_addr = inet_addr(ip);
	}
	else {
		sin.sin_addr.s_addr = INADDR_ANY;
	}

#endif

	int ret = bind(m_sock, (sockaddr*)&sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
#ifdef _WIN32
		printf("����,������˿�<%d>ʧ��...����%d\n", port, GetLastError());
#else
		perror("������˿�ʧ��:");
#endif
	}
	else {
		printf("������˿�<%d>�ɹ�...\n", port);
	}
	return ret;
}

int EasyServer::Listen(int n)
{
	int ret = listen(m_sock, n);
	if (SOCKET_ERROR == ret)
	{
#ifdef _WIN32
		printf("socket=<%d>����,��������˿�ʧ��...����%d\n", m_sock,GetLastError());
#else
		perror("��������˿�ʧ��:");
#endif
	}
	else {
		printf("socket=<%d>��������˿ڳɹ�...\n",m_sock);
	}
	return ret;

}

bool EasyServer::Run()
{
	if (isRun())
	{
		//�һ��selectģ��
		fd_set fdRead;
		SOCKET sMax = m_sock;

		//while (true)
		//{
			FD_ZERO(&fdRead);
			FD_SET(m_sock, &fdRead);
			
			for (auto it = m_clientFd.begin(); it != m_clientFd.begin();it++) {
				FD_SET(it->first, &fdRead);
			}
			timeval tTimeout = { 0,0 };

			int nReady = select(sMax + 1, &fdRead, NULL, NULL, &tTimeout);

			if (nReady < 0)
			{
				printf("���������������\n");
				Close();
				return false;
			}
			else if (nReady == 0) {
				return false;
				//continue;
			}

			if (FD_ISSET(m_sock, &fdRead))
			{
				//����������
				SOCKET cSock = Accept();
				if (cSock <= 0)
					return false;//continue;
				if (cSock > sMax)
					sMax = cSock;

				if (--nReady == 0)
					return true; //continue;
			}

			for (auto it = m_clientFd.begin(); it != m_clientFd.begin(); it++) {
				if (FD_ISSET(it->first, &fdRead))
				{
					//��������
					RecvData(it->first);

					if (--nReady == 0)
						break;
				}
			}

			return true;
		//}

	}
	else
	{
		return false;
	}
	
}

bool EasyServer::isRun()
{
	return m_sock != INVALID_SOCKET;
}

SOCKET EasyServer::Accept()
{
	SOCKET cSock = INVALID_SOCKET;
	sockaddr_in clientAddr = { 0 };
	int nAddrlen = sizeof(sockaddr_in);

#ifdef _WIN32
	cSock = accept(m_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
	cSock = accept(m_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
	if (cSock <= 0) {
#ifdef _WIN32
		printf("socket=<%d>����,���ܵ���Ч�ͻ���SOCKET...����%d\n", m_sock, GetLastError());
#else
		perror("���ܵ���Ч�ͻ���SOCKET:");
#endif
		return -1;
	}
	else {
		//�������ͻ��˷����յ����û���¼����Ϣ
		NewUserJoin newUser = { };
		newUser.scok = cSock;
		SendDataToAll(&newUser);
		AddClient(cSock);
		printf("socket=<%d>�յ��¿ͻ��˼��룺socket = %d,IP = %s ,port = %d\n", m_sock, cSock, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	}
	return cSock;
}

int EasyServer::RecvData(SOCKET cSock)
{
	int nLen = recv(cSock, m_chRecvBuff, sizeof(m_chRecvBuff), 0);
	DataHeader* header = (DataHeader*)m_chRecvBuff;
	if (nLen <= 0)
	{
		printf("�ͻ���<socket=%d>���˳����������\n", cSock);
		CloseSock(cSock);
		return -1;
	}

	ClientSocket* client = GetClient(cSock);
	if (!client)
		return -1;

	//����ȡ�������ݿ�������Ϣ��������β��,���յ����ٿ�����ȥ����
	memcpy(client->GetMsgBuf() + client->GetLastPos(), m_chRecvBuff, nLen);
	//��Ϣ������������β��λ�ú���
	client->SetLastPos(client->GetLastPos() + nLen);

	//ֻҪ��Ϣ�������е����ݣ����ȴ�����ϢͷDataHeader����
	while (client->GetLastPos() >= sizeof(DataHeader))
	{
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		DataHeader* header = (DataHeader*)client->GetMsgBuf();
		//��Ϣ�������е����ݳ��ȣ����ڵ��ڵ�ǰ���ݵĳ���
		if (client->GetLastPos() >= header->dataLength)
		{
			//��Ϊ����memcpy֮��header�����ݾͱ��ˣ���Ӧ��header->dataLengthҲ�ͱ��ˣ����ƶ�nLastPos��ʱ������
			//�����ȼ�¼����
			//��Ϣ����δ�������Ϣ����
			int size = client->GetLastPos() - header->dataLength;
			//������Ϣ
			OnNetMsg(client->GetSocket(),header);
			//����Ϣ��������ȥ��������Ϣ����������Ϣ֮��������Ƶ�������ͷ����
			memcpy(client->GetMsgBuf(), client->GetMsgBuf() + header->dataLength, size);
			//�ƶ���Ϣ������β��λ��
			client->SetLastPos(size);
		}
		else
		{
			//��Ϣ������ʣ�����ݲ���һ��������Ϣ
			break;
		}
	}
	return 0;
}

void EasyServer::OnNetMsg(SOCKET cSock,DataHeader* header)
{
	m_recvCount++;	//���յ�����Ϣ����+1
	auto t1 = m_tTime.getElapsedSecond();
	//��¼һ�����ڣ��յ��˶�������Ϣ
	if (t1 > 1.0)
	{
		//��ʱ�����Ϊ��ǰʱ�䣬��������Ϊ0
		m_tTime.update();
		m_recvCount = 0;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		struct Login* login = (Login*)header;
		printf("�յ��ͻ���<socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName = %s,passWord = %s\n", cSock, login->dataLength, login->userName, login->PassWord);
		struct LoginResult loginResult;
		strcpy(loginResult.data, "��¼�ɹ�");
		SendData(cSock, &loginResult);
	}
	break;
	case CMD_LOGOUT:
	{
		struct Logout* logout = (Logout*)header;
		printf("�յ��ͻ���<socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName = %s\n", cSock, logout->dataLength, logout->userName);
		struct LogoutResult logoutResult;
		SendData(cSock, &logoutResult);
	}
	break;
	case CMD_EXIT:
	{
		struct ExitConnect* cmd_exit = (ExitConnect*)header;
		printf("�յ��ͻ���<socket=%d>����CMD_EXIT,���ݳ��ȣ�%d\n", cSock, cmd_exit->dataLength);
		struct LogoutResult logoutResult;
		SendData(cSock, &logoutResult);
	}

	default:
		break;
	}

}

int EasyServer::SendData(SOCKET cSock, DataHeader* header)
{
	if (isRun() && header != NULL) {
		return send(cSock, (const char*)header, header->dataLength, 0);
	}
	return SOCKET_ERROR;
}

void EasyServer::SendDataToAll(DataHeader* data)
{
	for (auto it = m_clientFd.begin(); it != m_clientFd.begin(); it++) {
		SendData(it->first, data);
	}
}
void EasyServer::Close()
{
	for (auto it = m_clientFd.begin(); it != m_clientFd.begin(); it++) {
		CloseSock(it->first);
	}
	CloseSock(m_sock);
	m_clientFd.clear();
#ifdef _WIN32
	//���Windows socket����
	WSACleanup();
#endif

}

void EasyServer::CloseSock(SOCKET sock)
{
	if (sock != INVALID_SOCKET)
	{
		ClearClient(sock);

#ifdef _WIN32
		closesocket(sock);
		sock = INVALID_SOCKET;
#else
		close(sock);
		sock = INVALID_SOCKET;
#endif
	}
}

