#ifndef _CELLSEMAPHORE_H_
#define _CELLSEMAPHORE_H_
#include <chrono>
#include <thread>
#include <mutex>
//��������
#include <condition_variable>

using namespace std;
class CELLSemaphore
{
public:
	void wait();
	void wakeup();

private:
	//�����ȴ�����������
	std::condition_variable _cv;
	std::mutex _mutex;
	//�ȴ�����
	int _wait = 0;
	//���Ѽ���
	int _wakeup = 0;
};
#endif // _CELLSEMAPHORE_H_
