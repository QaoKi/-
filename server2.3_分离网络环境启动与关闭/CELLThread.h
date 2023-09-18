#ifndef _CELL_THREAD_H_
#define _CELL_THREAD_H_
#include "CELLSemaphore.h"
#include <mutex>

using namespace std;

class CELLThread
{
private:
	//����һ������ֵΪvoid������ΪCELLThread*�ĺ���ָ��
	typedef std::function<void(CELLThread*)> EventCall;
public:

	//�����߳�,������ʱ��ע������¼�
	void Start(EventCall onCreate = nullptr, 
			   EventCall onRun = nullptr,
			   EventCall onDestory = nullptr);
	//�ر��߳�,��¶���ⲿ���ⲿ�����Ժ󣬹ر��߳�
	void Close();
	//�˳��̣߳������̵߳Ĺ���������
	void Exit();
	
	bool isRun();
public:
	//��װһ��Sleep����������ʹ��
	static void Sleep(time_t dt);
protected:
	//�߳�����ʱ�Ĺ�������
	//�̱߳�����ѭ�����У���Ҫʹ�����Լ�����
	void OnWork();

private:
	//ע���߳��¼�
	EventCall _onCreate;
	//�����¼�
	EventCall _onRun;
	//�����߳��¼�
	EventCall _onDestory;
	//�߳��Ƿ�����������
	bool  _isRun = false;
	//�����̵߳�����״̬
	CELLSemaphore _sem;

	//��֤����߳�������ʱͬ�������߳��п���һ���̵߳���Start()��һ������Close()
	mutex _mutex;

};
#endif // _CELL_THREAD_H_
