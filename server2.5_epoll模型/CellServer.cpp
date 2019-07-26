#include "CellServer.h"

CellServer::CellServer(int id)
{
	_id = id;
	_INetEvent = NULL;
	_TaskServerPtr = make_shared<CellTaskServer>();
	_oldTime = CELLTime::getNowTimeInMilliSec();;
}

//��̬��Ա����������������ڴ�ռ�
int CellServer::_recvMsgCount = 0;

CellServer::~CellServer()
{
	CELLLog::Info("CellServer%d.Close() begin\n",_id);
	Close();
	CELLLog::Info("CellServer%d.Close() end\n", _id);
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
	int epfd = epoll_create(FD_SETSIZE);
	if (epfd < 0)
	{
		perror("epoll_create error:");
		return false;
	}

	struct epoll_event ep_in;
	struct epoll_event ep_out[FD_SETSIZE];

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

		for (auto it = _clientArray.begin(); it != _clientArray.end();) {
			//��_clientExit��û���ҵ���˵������ͻ��˻�û���˳�
			auto it1 = _clientExit.find(it->first);
			if (it1 == _clientExit.end())
			{
				ep_in.events = EPOLLIN | EPOLLOUT | EPOLLERR;
				ep_in.data.fd = it->second->GetSocket();
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, it->second->GetSocket(), &ep_in) < 0)
				{
					perror("epoll_ctl error:");
				}
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
			CELLThread::Sleep(1);
			continue;
		}

		int nReady = epoll_wait(epfd, ep_out, FD_SETSIZE, 0);
		if (nReady < 0)
		{
			CELLLog::Info("CellServer.Run epoll exit\n");
			pThread->Exit();
			break;
		}
		else if (nReady == 0) {
			checkHeart();
			CELLThread::Sleep(10);
			continue;
		}
		dealMsg(epfd, ep_out,nReady);
	}

	CELLLog::Info("CellServer%d.Run()  exit\n", _id);
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

void CellServer::dealMsg(int epfd, epoll_event* ep_out, int nReady)
{
	auto nowTime = CELLTime::getNowTimeInMilliSec();
	auto dt = nowTime - _oldTime;
	_oldTime = nowTime;
	map<SOCKET, ClientSocketPtr> clientTemp(_clientArray);

	for (int i = 0; i < nReady; i++)
	{
		clientTemp.erase(ep_out[i].data.fd);

		ClientSocketPtr client = GetClient(ep_out[i].data.fd);
		//���¼�
		if (ep_out[i].events & EPOLLIN)
		{
			//�����������ʱ�䣬���ܷ��������Ƿ��ǶϿ����ӵ���Ϣ
			client->resetDTHeart();
			//��������
			RecvData(client);
		}
		
		if (ep_out[i].events & EPOLLOUT)
		{
			//�����������ʱ�䣬�ܽ�������˵��Ҳ����
			client->resetDTHeart();
			//��д�¼�����������
			SendData(client);
		}

		if (ep_out[i].events & EPOLLERR)
		{
			//����յ��쳣�¼����Ͱ��տɶ��¼��Կͻ��˶����ݣ�
			//��Ϊ�ڶ����ݵĴ����У����û�ж������ݣ�����Ϊ�ͻ����˳�
			RecvData(client);
		}
		//�������¼��Ժ󣬴Ӻ������ժ����
		epoll_ctl(epfd, EPOLL_CTL_DEL, ep_out[i].data.fd, nullptr);
	}

	//���ʱ��clientTemp��ʣ�µĿͻ��˾���û�з���Ϣ�����ģ������������ʱ��
	for (auto it = clientTemp.begin(); it != clientTemp.end(); it++)
	{
		ClientSocketPtr client = GetClient(it->first);
		//�������
		//�ͻ����Ƿ��Ѿ���������ʱ��
		if (client->checkHeart(dt))
		{
			CELLLog::Info("client <socket=%d> heart dead\n", client->GetSocket());
			_INetEvent->ClientLeaveEvent(client);
			_clientExit.insert(client->GetSocket());
		}
	}
}

int CellServer::RecvData(ClientSocketPtr& pClient)
{
	if (!pClient)
		return -1;

	//�������ݵ��ͻ��˵Ľ��ջ�����
	int nLen = pClient->RecvData();
	//������ͻ����˳�
	if (nLen == SOCKET_ERROR)
	{
		CELLLog::Info("client <socket=%d> heart dead\n", pClient->GetSocket());
		_INetEvent->ClientLeaveEvent(pClient);
		_clientExit.insert(pClient->GetSocket());
		return -1;
	}

	//���ͻ��˽��ջ������е����ݣ����ȴ��ڵ���һ����������Ϣʱ
	//ѭ��������Ϣ
	while (pClient->hasMsg())
	{
		//������Ϣ
		OnNetMsg(pClient, pClient->front_msg());
		//��Ϣ��������Ժ��Ƴ���������ǰ��һ����Ϣ
		pClient->pop_front_msg();
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

	//ֱ�ӽ��ͻ��˷��ͻ������е����ݷ����ͻ���
	if(client->SendBuffReal() == SOCKET_ERROR)
	{
		//����ʧ�ܣ��˳�����ͻ���
		CELLLog::Info("client <socket=%d> SendData error\n", client->GetSocket());
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
		//CELLLog::Info("�յ��ͻ���<socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName = %s,passWord = %s\n", pClient->GetSocket(), login->dataLength, login->userName, login->PassWord);
		LoginResult* loginResult = new LoginResult;
		strcpy(loginResult->data, "login succ");
		addSendTask(pClient, (netmsg_DataHeader*)loginResult);
	}
	break;
	case CMD_HEART_C2S:
	{
		struct Heart_C2S* login = (Heart_C2S*)header;
		//CELLLog::Info("�յ��ͻ���<socket=%d>����CMD_HEART,���ݳ��ȣ�%d\n", pClient->GetSocket(), Heart_C2S->dataLength);
		pClient->resetDTHeart(); 
		//Heart_S2C* heart = new Heart_S2C;
		//addSendTask(pClient, (DataHeader*)heart);
	}
	break;
	case CMD_LOGOUT:
	{
		LogoutResult* logoutResult = new LogoutResult;
		strcpy(logoutResult->data, "logout succ");
		addSendTask(pClient, (netmsg_DataHeader*)logoutResult);
	}
	break;
	case CMD_EXIT:
	{
		struct ExitConnect* cmd_exit = (ExitConnect*)header;
		CELLLog::Info("recv client <socket=%d>request:CMD_EXIT,data length:%d\n", pClient->GetSocket(), cmd_exit->dataLength);
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
			CELLLog::Info("<Socket = %d> SendDataBuff full\n", pClient->GetSocket());
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
			CELLLog::Info("client <socket=%d> heart dead\n", it->second->GetSocket());
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