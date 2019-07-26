#include "CELLThread.h"

void CELLThread::Start(EventCall onCreate /*= nullptr*/, EventCall onRun /*= nullptr*/, EventCall onDestory /*= nullptr*/)
{
	//���ܶ���߳�ͬʱ����Start()
	lock_guard<std::mutex> lock(_mutex);

	if (!_isRun)
	{
		_isRun = true;

		if (onCreate)
			_onCreate = onCreate;
		if (onRun)
			_onRun = onRun;
		if (onDestory)
			_onDestory = onDestory;

		//&CELLThread::OnWork   ��Ա�����ĺ���ָ��
		//mem_fun()	����ȫ��ת��
		std::thread t(std::mem_fun(&CELLThread::OnWork), this);
		t.detach();
	}

}

void CELLThread::Close()
{
	lock_guard<std::mutex> lock(_mutex);
	if (_isRun)
	{
		_isRun = false;
		//�ȴ���������OnWork()�˳�
		_sem.wait();
	}
}

void CELLThread::Exit()
{
	lock_guard<std::mutex> lock(_mutex);
	if (_isRun)
	{
		_isRun = false;
		//���߳��ڲ��˳�������Ҫ�����ȴ���������ִ�����
	}
}

bool CELLThread::isRun()
{
	return _isRun;
}

void CELLThread::Sleep(time_t dt)
{
	chrono::milliseconds t(dt);	
	this_thread::sleep_for(t);
}

void CELLThread::OnWork()
{
	//EventCall�Ĳ�����CELLThread*����this����ȥ
	//��ע�ᣬ�����У��������
	if (_onCreate)
		_onCreate(this);

	if (_onRun)
		_onRun(this);

	if (_onDestory)
		_onDestory(this);

	_sem.wakeup();
}
