#include "CellServer.h"

CellServer::CellServer(int id)
{
	_id = id;
	_isRun = false;
	_pThread = NULL;
	_INetEvent = NULL;
	_TaskServerPtr = make_shared<CellTaskServer>();
	memset(_chRecvBuff, 0, sizeof(_chRecvBuff));
	_oldTime = CELLTime::getNowTimeInMilliSec();;
}

//��̬��Ա����������������ڴ�ռ�
int CellServer::_recvMsgCount = 0;

CellServer::~CellServer()
{
	if (_pThread)
	{
		delete _pThread;
		_pThread = NULL;
	}

	Close();
}
void CellServer::AddClientToBuff(ClientSocketPtr& pClient)
{
	//�Խ�������������γ��������Զ�����
	//lock_guard<mutex> lock(_clientBuffMutex);

	_clientBuffMutex.lock();
	_clientBuff.push_back(pClient);
	_clientBuffMutex.unlock();
}

void CellServer::AddClient(ClientSocketPtr& pClient)
{
	if (_clientArray.find(pClient->GetSocket()) == _clientArray.end())
	{
		_clientArray[pClient->GetSocket()] = pClient;
	}
}

ClientSocketPtr CellServer::GetClient(SOCKET cSock)
{
	auto it = _clientArray.find(cSock);
	if (it != _clientArray.end())
	{
		return it->second;
	}

	return NULL;
}

auto CellServer::ClearClient(SOCKET cSock)
{
	auto it = _clientArray.find(cSock);
	if (it != _clientArray.end())
	{
		return _clientArray.erase(it);
	}
	return _clientArray.end();
}

void CellServer::Start()
{
	//&CellServer::Run   ��Ա�����ĺ���ָ��
	//mem_fun()	����ȫ��ת��
	if (!_isRun)
	{
		_isRun = true;
		_pThread = new thread(mem_fun(&CellServer::Run), this);
		_pThread->detach();
		_TaskServerPtr->Start();
	}

}

//�õ�����������ݵķ������ж��ٸ��ͻ�������
int CellServer::GetClientNum()
{
	//�Ѿ����й���Ŀͻ��ˣ������߳��Ѿ���������û��ʼ����Ŀͻ���
	return _clientArray.size() + _clientBuff.size();
}

bool CellServer::Run()
{
	while (_isRun)
	{
		if (_clientBuff.size() > 0)
		{
			_clientBuffMutex.lock();
			for (auto pClient : _clientBuff)
			{
				AddClient(pClient);
			}
			_clientBuff.clear();
			_clientBuffMutex.unlock();
		}

		//�һ��selectģ��
		fd_set fdRead;
		SOCKET sMax = 0;

		FD_ZERO(&fdRead);

		for (auto it = _clientArray.begin(); it != _clientArray.end();) {
			//��_clientExit��û���ҵ���˵������ͻ��˻�û���˳�
			auto it1 = _clientExit.find(it->first);
			if (it1 == _clientExit.end())
			{
				sMax = sMax > it->second->GetSocket() ? sMax : it->second->GetSocket();
				FD_SET(it->first, &fdRead);
				it++;
			}
			else
			{
				//����ͻ����Ѿ��˳�
				it = ClearClient(it->first);
				_clientExit.erase(it1);
			}
		}

		if (_clientArray.size() == 0)
		{
			//����һ��������ʱʱ��
			_oldTime = CELLTime::getNowTimeInMilliSec();
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
			checkHeart();
			chrono::milliseconds t(1);	//1����
			this_thread::sleep_for(t);
			continue;
		}

		dealMsg(fdRead);
	}

	_sem.wakeup();
	return false;

}

void CellServer::SetEventObj(INetEvent* event)
{
	_INetEvent = event;
}

void CellServer::Close()
{
	if (_isRun)
	{
		_isRun = false;
		_TaskServerPtr->Close();
		//�����ȴ����߳̽���
		_sem.wait();
		_clientArray.clear();
		_clientBuff.clear();
		_clientExit.clear();
	}

}

void CellServer::dealMsg(fd_set& fdRead)
{
	auto nowTime = CELLTime::getNowTimeInMilliSec();
	auto dt = nowTime - _oldTime;
	_oldTime = nowTime;

	for (auto it = _clientArray.begin(); it != _clientArray.end(); it++)
	{
		ClientSocketPtr client = GetClient(it->first);

		//��ʱ������Ϣ���
		client->checkSendBuff(dt);

		//����յ��ͻ��˷�����������
		if (FD_ISSET(it->first, &fdRead))
		{
			//�����������ʱ�䣬���ܷ��������Ƿ��ǶϿ����ӵ���Ϣ
			client->resetDTHeart();
			//��������
			RecvData(client);
		}
		else
		{
			//�������
			//�ͻ����Ƿ��Ѿ���������ʱ��
			if (client->checkHeart(dt))
			{
				printf("�ͻ���<socket=%d>���˳����������\n", client->GetSocket());
				_INetEvent->ClientLeaveEvent(client);
				_clientExit.insert(client->GetSocket());
			}
		}
	}
}

int CellServer::RecvData(ClientSocketPtr& client)
{
	if (!client)
		return -1;

	int nLen = recv(client->GetSocket(), _chRecvBuff, sizeof(_chRecvBuff), 0);
	DataHeader* header = (DataHeader*)_chRecvBuff;
	if (nLen <= 0)
	{
		printf("�ͻ���<socket=%d>���˳����������\n", client->GetSocket());
		_INetEvent->ClientLeaveEvent(client);
		_clientExit.insert(client->GetSocket());
		return -1;
	}

	//����ȡ�������ݿ�������Ϣ��������β��,���յ����ٿ�����ȥ����
	memcpy(client->GetMsgBuf() + client->GetLastPos(), _chRecvBuff, nLen);
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

void CellServer::OnNetMsg(ClientSocketPtr& pClient, DataHeader* header)
{
	CellServer::_recvMsgCount++;	//���յ�����Ϣ����+1
	//auto t1 = _tTime.getElapsedSecond();
	////��¼һ�����ڣ��յ��˶�������Ϣ
	//if (t1 > 1.0)
	//{
	//	//��ʱ�����Ϊ��ǰʱ�䣬��������Ϊ0
	//	_tTime.update();
	//	_recvMsgCount = 0;
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
	case CMD_HEART_C2S:
	{
		struct Heart_C2S* login = (Heart_C2S*)header;
		//printf("�յ��ͻ���<socket=%d>����CMD_HEART,���ݳ��ȣ�%d\n", pClient->GetSocket(), Heart_C2S->dataLength);
		pClient->resetDTHeart(); 
		//Heart_S2C* heart = new Heart_S2C;
		//addSendTask(pClient, (DataHeader*)heart);
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

void CellServer::addSendTask(ClientSocketPtr pClient, DataHeader* header)
{
	//auto task = (CellTask*)std::make_shared<CellSendMsg2ClientTask>(pClient, header);
	//����һ��lambda���ʽ����ȥ
	/*
		ԭ���ķ�ʽ�Ǵ���һ��task�Ļ��࣬����������࣬���緢���������񣬼̳�
		������࣬��д�����doTask()������������ʵ��
		����ֱ����lambda���ʽ���ѷ����������doTask()������ŵ�lambda���ʽ��
	*/
	auto task = [pClient, header]() {

		pClient->SendData(header);
		//ִ���������Ժ󣬰�����ɾ�������ǿͻ��˲���ɾ��
		if (header)
		{
			delete header;
		}
	};
	_TaskServerPtr->AddTask(task);
}

void CellServer::checkHeart()
{
	auto nowTime = CELLTime::getNowTimeInMilliSec();
	auto dt = nowTime - _oldTime;
	_oldTime = nowTime;

	for (auto it = _clientArray.begin(); it != _clientArray.end(); it++)
	{
		if (it->second->checkHeart(dt))
		{
			printf("�ͻ���<socket=%d>���˳����������\n", it->second->GetSocket());
			_INetEvent->ClientLeaveEvent(it->second);
			_clientExit.insert(it->first);
		}
	}
}


//void CellServer::SendDataToAll(DataHeader* data)
//{
//	for (auto it = _clientArray.begin(); it != _clientArray.end(); it++) {
//		SendData(it->first, data);
//	}
//}