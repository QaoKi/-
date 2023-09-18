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
	//Ҫ����ó�Ա��������Ҫ����CellTaskServer����
	_thread.Start(
		//onCreate
		nullptr,
		//onRun
		[this](CELLThread* pThread) {
		Run(pThread);
	}, 
		//onClose
		nullptr);
}

void CellTaskServer::Close()
{
	_thread.Close();
}

void CellTaskServer::Run(CELLThread* pThread)
{
	//��ΪRun()�����У���Ҫ�ж��̵߳�.isRun()������������̣߳�����ط��Ͳ��ô����ˣ�
	//�����߳�ͨ�������������ж�
	while (pThread->isRun())
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
}
