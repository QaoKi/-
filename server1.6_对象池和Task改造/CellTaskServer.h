#ifndef _CellTaskServer_H
#define _CellTaskServer_H

#include <functional>
#include <thread>
#include <mutex>
#include <list>

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

private:
	//��������
	void Run();
	thread* m_pThread;
	//��������,������lambda���ʽ������ֱ��ִ��
	list<CellTask> m_listTask;
	//�������ݻ�����
	list<CellTask> m_listTaskBuf;
	mutex			m_mutex;
};
#endif