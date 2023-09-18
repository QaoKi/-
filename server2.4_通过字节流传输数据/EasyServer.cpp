#include "EasyServer.h"


EasyServer::EasyServer()
{
	_sock = INVALID_SOCKET;
	nRecvMsgCoutn = 0;
	//memset(_chRecvBuff, 0, sizeof(_chRecvBuff));
}


EasyServer::~EasyServer()
{
	Close();
}

void EasyServer::AddClient(ClientSocketPtr& pClient)
{
	lock_guard<std::mutex> lock(_clientArrayMutex);

	if (_clientArray.find(pClient->GetSocket()) == _clientArray.end())
	{
		_clientArray[pClient->GetSocket()] = pClient;
	}
}

void EasyServer::ClearClient(SOCKET cSock)
{
	auto it = _clientArray.find(cSock);
	if (it != _clientArray.end())
	{
		_clientArray.erase(it);
	}
}

void EasyServer::InitSocket()
{
	//��ʼ�����绷��
	CELLNetWork::Init();
	if (INVALID_SOCKET != _sock)
	{
		CELLLog::Info("<socket=%d>Close Old Socket...\n",(int)_sock);
		Close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == _sock)
	{
#ifdef _WIN32
		CELLLog::Info("error,create socket fail...error code:%d\n", GetLastError());
#else
		perror("create socket fail:\n");
#endif
	}
	else {
		CELLLog::Info("create socket=<%d> succ...\n", (int)_sock);
	}

	//�˿ڸ���
	int opt = 1;
	setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
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
	int ret = ::bind(_sock, (sockaddr*)&sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
#ifdef _WIN32
		CELLLog::Info("error,bind port<%d> fail...error code:%d\n", port, GetLastError());
#else
		perror("bind port fail:\n");
#endif
	}
	else {
		CELLLog::Info("bind port<%d> succ...\n", port);
	}
	return 0;
}

int EasyServer::Listen(int n)
{
	int ret = listen(_sock, n);
	if (SOCKET_ERROR == ret)
	{
#ifdef _WIN32
		CELLLog::Info("socket=<%d> error,listen port fail...error code:%d\n", _sock,GetLastError());
#else
		perror("listen port fail:\n");
#endif
	}
	else {
		CELLLog::Info("socket=<%d>listen port succ...\n",_sock);
	}
	return ret;

}

void EasyServer::Start(int nCellServerCount)
{
	for (int i = 0; i < nCellServerCount; i++)
	{
		auto pSer = std::make_shared<CellServer>(i+1);
		_CellServerArray.push_back(pSer);
		pSer->Start();
		pSer->SetEventObj(this);
	}

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

bool EasyServer::Run(CELLThread* pThread)
{

	//�һ��selectģ��
	while (pThread->isRun())
	{			
		Time4Msg();
		fd_set fdRead;
		SOCKET sMax = _sock;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		timeval tTimeout = { 0,0 };
		int nReady = select(sMax + 1, &fdRead, NULL, NULL, &tTimeout);

		if (nReady < 0)
		{
			CELLLog::Info("server select exit\n");
			pThread->Exit();
			break;
		}
		else if (nReady == 0) {
			CELLThread::Sleep(1);
			continue;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			//����������
			Accept();
		}

	}

	CELLLog::Info("EasyServer.Run()  exit\n");
	return true;
}

bool EasyServer::isRun()
{
	return _sock != INVALID_SOCKET;
}

void EasyServer::ClientLeaveEvent(ClientSocketPtr& pClient)
{
	ClearClient(pClient->GetSocket());
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
	cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
	cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
	if (cSock <= 0) {
#ifdef _WIN32
		CELLLog::Info("socket=<%d>error,accept invalid client SOCKET...error code:%d\n", _sock, GetLastError());
#else
		perror("accept invalid client SOCKET:\n");
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

		//CELLLog::Info("socket=<%d>�յ��¿ͻ��˼��룺socket = %d,IP = %s ,port = %d\n", _sock, cSock, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		//CELLLog::Info("socket=<%d>�յ��¿ͻ��˼��룺socket = %d,�ͻ���������%d\n", _sock, cSock, _clientArray.size());
	}
	return cSock;
}

void EasyServer::AddClientToCellServer(ClientSocketPtr& pClient)
{
	//�����ͻ��������ٵ�CellServer
	//�����һ�������浽��һ��
	auto pMinNumClient = _CellServerArray[0];
	for (size_t i = 1; i < _CellServerArray.size(); i++)
	{
		pMinNumClient = pMinNumClient->GetClientNum() > _CellServerArray[i]->GetClientNum() ? _CellServerArray[i] : pMinNumClient;
	}

	pMinNumClient->AddClientToBuff(pClient);
}

void EasyServer::Time4Msg()
{
	auto t1 = _tTime.getElapsedSecond();
	//��¼һ�����ڣ��յ��˶�������Ϣ
	if (t1 > 1.0)
	{
		//��ʱ�����Ϊ��ǰʱ�䣬��������Ϊ0
		_tTime.update();
		CELLLog::Info("socket=<%d>,time:%lf,client num:%d,msg num:%d\n", _sock, t1, _clientArray.size(),CellServer::_recvMsgCount);
		CellServer::_recvMsgCount = 0;
	}

}

void EasyServer::Close()
{
	CELLLog::Info("EasyServer.Close() begin\n");
	_thread.Close();
	if (_sock != INVALID_SOCKET)
	{
		_CellServerArray.clear();
		_clientArray.clear();

#ifdef _WIN32
		closesocket(_sock);
#else
		close(_sock);

#endif
		_sock = INVALID_SOCKET;
	}
	CELLLog::Info("EasyServer.Close() end\n");
}


