#ifndef _CellTaskServer_H
#define _CellTaskServer_H

#include <functional>
#include <thread>
#include <mutex>
#include <list>
#include "CELLSemaphore.h"
#include "CELLThread.h"

using namespace std;

class CellTaskServer;
typedef std::shared_ptr<CellTaskServer> CellTaskServerPtr;

//ִ������ķ���
class CellTaskServer
{
	typedef std::function<void()> CellTask;
public:
	CellTaskServer();
	~CellTaskServer();

	void AddTask(CellTask task);
	void Start();
	void Close();

private:
	CELLThread _thread;
	//��������
	void Run(CELLThread* pThread);
	//��������,������lambda���ʽ������ֱ��ִ��
	list<CellTask> _listTask;
	//�������ݻ�����
	list<CellTask> _listTaskBuf;
	mutex			_mutex;
};
#endif