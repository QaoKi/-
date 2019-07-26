#ifndef _CellTaskServer_H
#define _CellTaskServer_H

#include "CellTask.h" 

class CellTaskServer;
typedef std::shared_ptr<CellTaskServer> CellTaskServerPtr;

//ִ������ķ���
class CellTaskServer
{
public:
	CellTaskServer();
	~CellTaskServer();

	void AddTask(CellTaskPtr& task);
	void Start();

private:
	//��������
	void Run();
	thread* m_pThread;
	//��������
	list<CellTaskPtr> m_listTask;
	//�������ݻ�����
	list<CellTaskPtr> m_listTaskBuf;
	mutex			m_mutex;
};
#endif