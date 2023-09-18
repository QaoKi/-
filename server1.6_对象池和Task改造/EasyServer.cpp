#include "EasyServer.h"


EasyServer::EasyServer()
{
	m_sock = INVALID_SOCKET;
	nRecvMsgCoutn = 0;
	//memset(m_chRecvBuff, 0, sizeof(m_chRecvBuff));
}


EasyServer::~EasyServer()
{
	//Close();
	closesocket(m_sock);
	//for (auto pCellServer : m_CellServerArray)
	//{
	//	if (pCellServer)
	//	{
	//		delete pCellServer;
	//		pCellServer = NULL;
	//	}
	//}
	m_CellServerArray.clear();
	m_clientArray.clear();
	
}

void EasyServer::AddClient(ClientSocketPtr& pClient)
{
	if (m_clientArray.find(pClient->GetSocket()) == m_clientArray.end())
	{
		if(pClient)
			m_clientArray[pClient->GetSocket()] = pClient;
	}
}

ClientSocketPtr EasyServer::GetClient(SOCKET cSock)
{
	auto it = m_clientArray.find(cSock);
	if (it != m_clientArray.end())
	{
		return it->second;
	}

	return NULL;
}

void EasyServer::ClearClient(SOCKET cSock)
{
	auto it = m_clientArray.find(cSock);
	if (it != m_clientArray.end())
	{
		//if (it->second)
		//{
		//	delete it->second;
		//	it->second = NULL;
		//}
			
		m_clientArray.erase(it);
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
		//Close();
		closesocket(m_sock);
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

	//c++11�е�bind������socket�е�bind��һ�������ʹ����std�������ռ䣬Ĭ�ϻ�ʹ��c++11�е�bind
	//������bindǰ�����::����ʾʹ��socket��bind
	int ret = ::bind(m_sock, (sockaddr*)&sin, sizeof(sockaddr_in));
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
	return 0;
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

void EasyServer::Start(int nCellServerCount)
{
	for (int i = 0; i < nCellServerCount; i++)
	{
		auto pSer = std::make_shared<CellServer>(m_sock);
		m_CellServerArray.push_back(pSer);
		pSer->Start();
		pSer->SetEventObj(this);
	}
	Run();
}

bool EasyServer::Run()
{

	//�һ��selectģ��
	fd_set fdRead;
	SOCKET sMax = m_sock;

	timeval tTimeout = { 0,0 };

	while (isRun())
	{			
		Time4Msg();
		FD_ZERO(&fdRead);
		FD_SET(m_sock, &fdRead);

		int nReady = select(sMax + 1, &fdRead, NULL, NULL, &tTimeout);

		if (nReady < 0)
		{
			printf("���������������\n");
			CloseSock(m_sock);
			return false;
		}
		else if (nReady == 0) {
			chrono::milliseconds t(1);
			this_thread::sleep_for(t);
			continue;
		}

		if (FD_ISSET(m_sock, &fdRead))
		{
			//����������
			Accept();
		}

	}
	return true;
}

bool EasyServer::isRun()
{
	return m_sock != INVALID_SOCKET;
}

void EasyServer::ClientLeaveEvent(ClientSocketPtr& pClient)
{
	CloseSock(pClient->GetSocket());
}

void EasyServer::RecvMsgEvent(ClientSocketPtr& pClient)
{
	nRecvMsgCoutn++;
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
		//NewUserJoin newUser = { };
		//newUser.scok = cSock;
		//SendDataToAll(&newUser);

		//������ָ�룬�����make_shared���죬������ö����
		//auto pClient = std::make_shared<ClientSocket>(cSock);
		ClientSocketPtr pClient(new ClientSocket(cSock));
		AddClient(pClient);
		//������ͻ��ˣ�����CellServer�������ݹ���
		AddClientToCellServer(pClient);

		//printf("socket=<%d>�յ��¿ͻ��˼��룺socket = %d,IP = %s ,port = %d\n", m_sock, cSock, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		//printf("socket=<%d>�յ��¿ͻ��˼��룺socket = %d,�ͻ���������%d\n", m_sock, cSock, m_clientArray.size());
	}
	return cSock;
}

void EasyServer::AddClientToCellServer(ClientSocketPtr& pClient)
{
	//�����ͻ��������ٵ�CellServer
	//�����һ�������浽��һ��
	auto pMinNumClient = m_CellServerArray[0];
	for (size_t i = 1; i < m_CellServerArray.size(); i++)
	{
		pMinNumClient = pMinNumClient->GetClientNum() > m_CellServerArray[i]->GetClientNum() ? m_CellServerArray[i] : pMinNumClient;
	}

	pMinNumClient->AddClientToBuff(pClient);
}

void EasyServer::Time4Msg()
{
	auto t1 = m_tTime.getElapsedSecond();
	//��¼һ�����ڣ��յ��˶�������Ϣ
	if (t1 > 1.0)
	{
		//��ʱ�����Ϊ��ǰʱ�䣬��������Ϊ0
		m_tTime.update();
		printf("socket=<%d>,ʱ�䣺%lf,�ͻ���������%d,��Ϣ������%d\n", m_sock, t1, m_clientArray.size(),CellServer::m_recvMsgCount);
		CellServer::m_recvMsgCount = 0;
	}

}

//void EasyServer::Close()
//{
//	for (auto it = m_clientArray.begin(); it != m_clientArray.end(); it++) {
//		CloseSock(it->first);
//	}
//	CloseSock(m_sock);
//	m_clientArray.clear();
//#ifdef _WIN32
//	//���Windows socket����
//	WSACleanup();
//#endif
//
//}

void EasyServer::CloseSock(SOCKET sock)
{
	if (sock != INVALID_SOCKET)
	{
		ClearClient(sock);
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = INVALID_SOCKET;
	}
}

