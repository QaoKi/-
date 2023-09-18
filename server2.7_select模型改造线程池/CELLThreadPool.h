#ifndef _CELL_THREAD_POOL_H_
#define _CELL_THREAD_POOL_H_

#include <functional>
#include <list>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include "CELLSemaphore.h"
#include "CELLThread.h"
using namespace std;

class CELLThreadPool
{
	typedef std::function<void()> CellTask;
public:
	CELLThreadPool(int threadCount);
	~CELLThreadPool();

	void AddTask(CellTask task);

private:
	void Run();
	void adjustThread();
	vector<thread> _vecThreads;
	thread* _adjust;	//	�������߳�

	CELLSemaphore _task_empty;	//	û��������
	int _threadCount;
	//��������,������lambda���ʽ������ֱ��ִ��
	list<CellTask> _listTask;
	//�������ݻ�����
	list<CellTask> _listTaskBuf;
	mutex	_mutexTaskBuff;
	mutex	_mutexTaskList;
	
};

#endif

