#include "CellServer.h"

CellServer::CellServer(SOCKET sock)
{
	m_sock = sock;
	m_pThread = NULL;
	m_INetEvent = NULL;
	m_TskServer = new CellTaskServer();
	memset(m_chRecvBuff, 0, sizeof(m_chRecvBuff));
}

//��̬��Ա����������������ڴ�ռ�
int CellServer::m_recvMsgCount = 0;

CellServer::~CellServer()
{
	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = NULL;
	}
	if (m_TskServer)
	{
		delete m_TskServer;
		m_TskServer = NULL;
	}
	m_INetEvent = NULL;
	memset(m_chRecvBuff, 0, sizeof(m_chRecvBuff));
	m_clientArray.clear();
	m_clientBuff.clear();
}
void CellServer::AddClientToBuff(ClientSocket* pClient)
{
	//�Խ�������������γ��������Զ�����
	//lock_guard<mutex> lock(m_clientBuffMutex);

	m_clientBuffMutex.lock();
	m_clientBuff.push_back(pClient);
	m_clientBuffMutex.unlock();
}

void CellServer::AddClient(ClientSocket* pClient)
{
	if (m_clientArray.find(pClient->GetSocket()) == m_clientArray.end())
	{
		if (pClient)
			m_clientArray[pClient->GetSocket()] = pClient;
	}
}

ClientSocket* CellServer::GetClient(SOCKET cSock)
{
	auto it = m_clientArray.find(cSock);
	if (it != m_clientArray.end())
	{
		return it->second;
	}

	return NULL;
}

auto CellServer::ClearClient(SOCKET cSock)
{
	auto it = m_clientArray.find(cSock);
	if (it != m_clientArray.end())
	{
		return m_clientArray.erase(it);
	}
	return m_clientArray.end();
}

void CellServer::Start()
{
	//&CellServer::Run   ��Ա�����ĺ���ָ��
	//mem_fun()	����ȫ��ת��
	m_pThread = new thread(mem_fun(&CellServer::Run), this);
	m_pThread->detach();
	m_TskServer->Start();
}

//�õ�����������ݵķ������ж��ٸ��ͻ�������
int CellServer::GetClientNum()
{
	//�Ѿ����й���Ŀͻ��ˣ������߳��Ѿ���������û��ʼ����Ŀͻ���
	return m_clientArray.size() + m_clientBuff.size();
}

bool CellServer::Run()
{
	while (isRun())
	{
		if (m_clientBuff.size() > 0)
		{
			m_clientBuffMutex.lock();
			for (auto pClient : m_clientBuff)
			{
				AddClient(pClient);
			}
			m_clientBuff.clear();
			m_clientBuffMutex.unlock();
		}

		//�һ��selectģ��
		fd_set fdRead;
		SOCKET sMax = 0;

		FD_ZERO(&fdRead);

		for (auto it = m_clientArray.begin(); it != m_clientArray.end();) {
			//��m_clientExit��û���ҵ���˵������ͻ��˻�û���˳�
			auto it1 = m_clientExit.find(it->first);
			if (it1 == m_clientExit.end())
			{
				sMax = sMax > it->second->GetSocket() ? sMax : it->second->GetSocket();
				FD_SET(it->first, &fdRead);
				it++;
			}
			else
			{
				//����ͻ����Ѿ��˳�
				it = ClearClient(it->first);
				m_clientExit.erase(it1);
			}
		}

		if (m_clientArray.size() == 0)
		{
			//��c++�����ߣ�����windows�ģ���Ȼ��ƽ̨��Ҫ�޸�
			chrono::milliseconds t(1);	//1����
			this_thread::sleep_for(t);
			continue;
		}
		timeval tTimeout = { 0,0 };

		int nReady = select(sMax + 1, &fdRead, NULL, NULL, &tTimeout);

		if (nReady < 0)
		{
			printf("select���������\n");
			Close();
			return false;
		}
		else if (nReady == 0) {
			chrono::milliseconds t(1);	//1����
			this_thread::sleep_for(t);
			continue;
		}

		for (auto it = m_clientArray.begin(); it != m_clientArray.end(); it++) {
			if (FD_ISSET(it->first, &fdRead))
			{
				//��������
				RecvData(it->first);
				if (--nReady == 0)
					break;
			}
		}

	}

	return false;

}

bool CellServer::isRun()
{
	return m_sock != INVALID_SOCKET;
}

void CellServer::SetEventObj(INetEvent* event)
{
	m_INetEvent = event;
}

void CellServer::Close()
{
	for (auto it = m_clientArray.begin(); it != m_clientArray.end(); it++) {
		CloseSock(it->first);
	}
	m_clientArray.clear();
#ifdef _WIN32
	//���Windows socket����
	WSACleanup();
#endif

}

void CellServer::CloseSock(SOCKET sock)
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

int CellServer::RecvData(SOCKET cSock)
{
	ClientSocket* client = GetClient(cSock);
	if (!client)
		return -1;

	int nLen = recv(cSock, m_chRecvBuff, sizeof(m_chRecvBuff), 0);
	DataHeader* header = (DataHeader*)m_chRecvBuff;
	if (nLen <= 0)
	{
		printf("�ͻ���<socket=%d>���˳����������\n", cSock);
		m_INetEvent->ClientLeaveEvent(client);
		m_clientExit.insert(cSock);
		return -1;
	}

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
			OnNetMsg(client, header);
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

void CellServer::OnNetMsg(ClientSocket* pClient, DataHeader* header)
{
	CellServer::m_recvMsgCount++;	//���յ�����Ϣ����+1
	//auto t1 = m_tTime.getElapsedSecond();
	////��¼һ�����ڣ��յ��˶�������Ϣ
	//if (t1 > 1.0)
	//{
	//	//��ʱ�����Ϊ��ǰʱ�䣬��������Ϊ0
	//	m_tTime.update();
	//	m_recvMsgCount = 0;
	//}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		//struct Login* login = (Login*)header;
		//printf("�յ��ͻ���<socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName = %s,passWord = %s\n", pClient->GetSocket(), login->dataLength, login->userName, login->PassWord);
		LoginResult* loginResult = new LoginResult;
		strcpy(loginResult->data, "��¼�ɹ�");
		addSendTask(pClient, (DataHeader*)loginResult);
	}
	break;
	case CMD_LOGOUT:
	{
		struct Logout* logout = (Logout*)header;
		printf("�յ��ͻ���<socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName = %s\n", pClient->GetSocket(), logout->dataLength, logout->userName);
		struct LogoutResult logoutResult;
	}
	break;
	case CMD_EXIT:
	{
		struct ExitConnect* cmd_exit = (ExitConnect*)header;
		printf("�յ��ͻ���<socket=%d>����CMD_EXIT,���ݳ��ȣ�%d\n", pClient->GetSocket(), cmd_exit->dataLength);
	}

	default:
		break;
	}

}

void CellServer::addSendTask(ClientSocket* pClient, DataHeader* header)
{
	CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
	m_TskServer->AddTask(task);
}

//void CellServer::SendDataToAll(DataHeader* data)
//{
//	for (auto it = m_clientArray.begin(); it != m_clientArray.end(); it++) {
//		SendData(it->first, data);
//	}
//}