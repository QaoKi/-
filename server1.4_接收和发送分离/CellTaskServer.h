#ifndef _CellTaskServer_H
#define _CellTaskServer_H

#include "CellTask.h"

//ִ������ķ���
class CellTaskServer
{
public:
	CellTaskServer();
	~CellTaskServer();

	void AddTask(CellTask* task);
	void Start();

private:
	//��������
	void Run();
	thread* m_pThread;
	//��������
	list<CellTask*> m_listTask;
	//�������ݻ�����
	list<CellTask*> m_listTaskBuf;
	mutex			m_mutex;
};
#endif