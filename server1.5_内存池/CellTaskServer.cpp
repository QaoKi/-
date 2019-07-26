#include "CellTaskServer.h"

CellTaskServer::CellTaskServer()
{
	m_pThread = NULL;
}

CellTaskServer::~CellTaskServer()
{
	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = NULL;
	}

}

void CellTaskServer::AddTask(CellTaskPtr& task)
{
	//�Խ���
	lock_guard<mutex> lock(m_mutex);
	m_listTaskBuf.push_back(task);
}

void CellTaskServer::Start()
{
	m_pThread = new thread(mem_fun(&CellTaskServer::Run), this);
	m_pThread->detach();
}

void CellTaskServer::Run()
{
	while (true)
	{
		//�����񻺳����а�����ȡ����
		if (m_listTaskBuf.size() > 0)
		{
			lock_guard<mutex> lock(m_mutex);
			for (auto pTask : m_listTaskBuf)
			{
				m_listTask.push_back(pTask);
			}
			m_listTaskBuf.clear();
		}

		//û������
		if (m_listTask.empty())
		{
			chrono::milliseconds t(1);
			this_thread::sleep_for(t);
			continue;
		}

		//��������
		for (auto pTask : m_listTask)
		{
			pTask->doTask();
			//if (pTask)
			//{
			//	delete pTask;
			//	pTask = NULL;
			//}
		}
		m_listTask.clear();
	}
}
