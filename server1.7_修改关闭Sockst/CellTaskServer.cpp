#include "CellTaskServer.h"

CellTaskServer::CellTaskServer()
{
}

CellTaskServer::~CellTaskServer()
{
	Close();
}

void CellTaskServer::AddTask(CellTask task)
{
	//�Խ���
	lock_guard<mutex> lock(_mutex);
	_listTaskBuf.push_back(task);
}

void CellTaskServer::Start()
{
	if (!_isRun)
	{
		_isRun = true;
		_pThread = new thread(mem_fun(&CellTaskServer::Run), this);
		_pThread->detach();
	}
}

void CellTaskServer::Close()
{
	if (_isRun)
	{
		_isRun = false;
		//�����ȴ����߳̽���
		_sem.wait();
	}
}

void CellTaskServer::Run()
{
	//��ΪRun()�����У���Ҫ�ж��̵߳�.isRun()������������̣߳�����ط��Ͳ��ô����ˣ�
	//�����߳�ͨ�������������ж�
	while (_isRun)
	{
		//�����񻺳����а�����ȡ����
		if (_listTaskBuf.size() > 0)
		{
			lock_guard<mutex> lock(_mutex);
			for (auto pTask : _listTaskBuf)
			{
				_listTask.push_back(pTask);
			}
			_listTaskBuf.clear();
		}

		//û������
		if (_listTask.empty())
		{
			chrono::milliseconds t(1);
			this_thread::sleep_for(t);
			continue;
		}

		//��������
		for (auto pTask : _listTask)
		{
			//ֱ��ִ����������
			pTask();
		}
		_listTask.clear();
	}
	//���߳̽�������������е�����϶�ִ�����ˣ��������񻺳����п��ܻ���ʣ������û�д���
	//�������񻺳����п��ܲ���������
	lock_guard<mutex> lock(_mutex);
	for (auto pTask : _listTaskBuf)
	{
		//ֱ��ִ����������
		pTask();
	}
	printf("CellTaskServer.Run()  exit\n");
	_sem.wakeup();
}
