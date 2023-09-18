#include "CellServer.h"

CellServer::CellServer(int id)
{
	_id = id;
	_INetEvent = NULL;
	_TaskServerPtr = make_shared<CellTaskServer>();
	memset(_chRecvBuff, 0, sizeof(_chRecvBuff));
	_oldTime = CELLTime::getNowTimeInMilliSec();;
}

//��̬��Ա����������������ڴ�ռ�
int CellServer::_recvMsgCount = 0;

CellServer::~CellServer()
{
	printf("CellServer%d.Close() begin\n",_id);
	Close();
	printf("CellServer%d.Close() end\n", _id);
}
void CellServer::AddClientToBuff(ClientSocketPtr& pClient)
{
	//�Խ�������������γ��������Զ�����
	lock_guard<mutex> lock(_clientBuffMutex);
	_clientBuff.push_back(pClient);
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
	_thread.Start(
		//onCreate
		nullptr,
		//onRun
		[this](CELLThread* pThread) {
		Run(pThread);
	}, 
		//onClose
		[this](CELLThread* pThread) {
		//������CellServer��Close(),�����̻߳��˳��Ժ󣬽��ͻ�������
		ClearAllClient();
	});

	_TaskServerPtr->Start();
}

//�õ�����������ݵķ������ж��ٸ��ͻ�������
int CellServer::GetClientNum()
{
	//�Ѿ����й���Ŀͻ��ˣ������߳��Ѿ���������û��ʼ����Ŀͻ���
	return _clientArray.size() + _clientBuff.size();
}

bool CellServer::Run(CELLThread * pThread)
{
	while (pThread->isRun())
	{
		if (_clientBuff.size() > 0)
		{
			lock_guard<std::mutex> lock(_clientBuffMutex);
			for (auto pClient : _clientBuff)
			{
				AddClient(pClient);
			}
			_clientBuff.clear();
		}

		//�һ��selectģ��
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExc;	//�쳣
		SOCKET sMax = 0;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExc);

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

		//������д�¼����쳣�¼����ͻ�����һ����
		memcpy(&fdWrite, &fdRead, sizeof(fd_set));
		memcpy(&fdExc, &fdRead, sizeof(fd_set));

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
		//����һ�����е�socket���鿴�Ƿ�ɶ�����д���쳣�¼�
		//��ΪҪ�����飬�������ܺܲ�쳣����������⣬�����쳣���Բ���
		int nReady = select(sMax + 1, &fdRead, &fdWrite, &fdExc, &tTimeout);

		if (nReady < 0)
		{
			printf("CellServer.Run select end\n");
			//��������Ҫ�˳��̣߳���������̵߳�Close()������_isRun��Ϊfalse,Ȼ���̻߳�������
			//���ǻ�������������ʱ�����߳�ִ����ϣ������߳��˳����ˣ������һ��������Exit()
			//pThread->Close();������
			pThread->Exit();
			break;
		}
		else if (nReady == 0) {
			checkHeart();
			chrono::milliseconds t(1);	//1����
			this_thread::sleep_for(t);
			continue;
		}
		dealMsg(fdRead,fdWrite,fdExc);
	}

	printf("CellServer%d.Run()  exit\n", _id);
	return false;

}

void CellServer::SetEventObj(INetEvent* event)
{
	_INetEvent = event;
}

void CellServer::Close()
{
	_thread.Close();
	_TaskServerPtr->Close();
}

void CellServer::ClearAllClient()
{
	_clientArray.clear();
	_clientBuff.clear();
	_clientExit.clear();
}

void CellServer::dealMsg(fd_set& fdRead, fd_set& fdWrite, fd_set& fdExc)
{
	auto nowTime = CELLTime::getNowTimeInMilliSec();
	auto dt = nowTime - _oldTime;
	_oldTime = nowTime;

	for (auto it = _clientArray.begin(); it != _clientArray.end(); it++)
	{
		ClientSocketPtr client = GetClient(it->first);

		//��Ϊ������ϢҪ�ͻ���socket�п�д�¼�������ֱ�ӷ��ͣ����Զ�ʱ������ע��
		//��ʱ������Ϣ���
		//client->checkSendBuff(dt);

		//����յ��ͻ��˷�����������
		if (FD_ISSET(it->first, &fdRead))
		{
			//�����������ʱ�䣬���ܷ��������Ƿ��ǶϿ����ӵ���Ϣ
			client->resetDTHeart();
			//��������
			RecvData(client);
		}
		else if (FD_ISSET(it->first, &fdWrite))
		{
			//�����������ʱ�䣬�ܽ�������˵��Ҳ����
			client->resetDTHeart();
			SendData(client);
		}
		//�쳣�¼�һ���ǿͻ��˲������¼�����Ҫ����������������жϿͻ����Ƿ�����
		else if (FD_ISSET(it->first, &fdExc))
		{
			//�����������ʱ�䣬�ܽ�������˵��Ҳ����
			//����յ��쳣�¼����Ͱ��տɶ��¼��Կͻ��˶����ݣ�
			//��Ϊ�ڶ����ݵĴ����У����û�ж������ݣ�����Ϊ�ͻ����˳�
			RecvData(client);
		}
		else
		{
			//�������
			//�ͻ����Ƿ��Ѿ���������ʱ��
			if (client->checkHeart(dt))
			{
				printf("client <socket=%d> heart dead\n", client->GetSocket());
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
	netmsg_DataHeader* header = (netmsg_DataHeader*)_chRecvBuff;
	if (nLen <= 0)
	{
		printf("client <socket=%d> heart dead\n", client->GetSocket());
		_INetEvent->ClientLeaveEvent(client);
		_clientExit.insert(client->GetSocket());
		return -1;
	}

	//����ȡ�������ݿ�������Ϣ��������β��,���յ����ٿ�����ȥ����
	memcpy(client->GetMsgBuf() + client->GetLastPos(), _chRecvBuff, nLen);
	//��Ϣ������������β��λ�ú���
	client->SetLastPos(client->GetLastPos() + nLen);

	//ֻҪ��Ϣ�������е����ݣ����ȴ�����ϢͷDataHeader����
	while (client->GetLastPos() >= sizeof(netmsg_DataHeader))
	{
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		netmsg_DataHeader* header = (netmsg_DataHeader*)client->GetMsgBuf();
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

int CellServer::SendData(ClientSocketPtr& client)
{
	//������ͻ��˿�д��ʱ��ֱ�Ӱѻ����������ݷ��ͳ�ȥ
	/*��Ҫע����ǣ���Ϊ��selectģ�ͣ�����һ���ͻ��˿ɶ��Ϳ�д�¼������ˣ����ȴ���ɶ��¼���
	���ݾ�������֮�󣬴���һ��Ҫ�ظ�����Ϣ�ŵ����ͻ���������ȥ�����д�¼����������������ݷ���ȥ��
	��Ϊ����ɶ��¼��Ϳ�д�¼����ǲ��еģ����Կͻ��˵ķ��ͻ��������̰߳�ȫ��
	*/
	if(client->SendBuffReal() < 0)
	{
		//����ʧ�ܣ��˳�����ͻ���
		printf("client <socket=%d> SendData error\n", client->GetSocket());
		_INetEvent->ClientLeaveEvent(client);
		_clientExit.insert(client->GetSocket());
		return -1;
	}

	return 0;
}

void CellServer::OnNetMsg(ClientSocketPtr& pClient, netmsg_DataHeader* header)
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
		strcpy(loginResult->data, "login succ");
		addSendTask(pClient, (netmsg_DataHeader*)loginResult);
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
		printf("recv client <socket=%d>request:CMD_LOGOUT,data length:%d,userName = %s\n", pClient->GetSocket(), logout->dataLength, logout->userName);
		struct LogoutResult logoutResult;
	}
	break;
	case CMD_EXIT:
	{
		struct ExitConnect* cmd_exit = (ExitConnect*)header;
		printf("recv client <socket=%d>request:CMD_EXIT,data length:%d\n", pClient->GetSocket(), cmd_exit->dataLength);
	}

	default:
		break;
	}

}

void CellServer::addSendTask(ClientSocketPtr pClient, netmsg_DataHeader* header)
{
	//auto task = (CellTask*)std::make_shared<CellSendMsg2ClientTask>(pClient, header);
	//����һ��lambda���ʽ����ȥ
	/*
		ԭ���ķ�ʽ�Ǵ���һ��task�Ļ��࣬����������࣬���緢���������񣬼̳�
		������࣬��д�����doTask()������������ʵ��
		����ֱ����lambda���ʽ���ѷ����������doTask()������ŵ�lambda���ʽ��
	*/
	auto task = [pClient, header]() {

		//�������������

		if (pClient->SendData(header) < 0)
		{
			printf("<Socket = %d> SendDataBuff full\n", pClient->GetSocket());
			//������ӱ�Ĺ��ܣ��������ݷŵ����ݿ����д�뱾���ļ�
		}
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
			printf("client <socket=%d> heart dead\n", it->second->GetSocket());
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