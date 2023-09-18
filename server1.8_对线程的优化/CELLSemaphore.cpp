#include "CELLSemaphore.h"


void CELLSemaphore::wait()
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (--_wait < 0)
	{
		//�����ȴ�
		/*
			wait()�ĵڶ�����������һ��Lambda���ʽ
			���һ�����ѵ�������ֻ�е�_wakeup > 0��ʱ����ܱ�����
			��ֹα���ѣ�ֻ��������wakeup()���ܻ���
		*/
	
		_cv.wait(lock, [this]() -> bool {
			return _wakeup > 0;
		});
		--_wakeup;
	}
		
}

void CELLSemaphore::wakeup()
{
	std::unique_lock<std::mutex> lock(_mutex);
	//���������߳�t1��t2ͬʱ������wake()����ʱ_wait��-2������Ҫ�ж� ++_wait<=0
	if (++_wait <= 0)
	{
		++_wakeup;
		_cv.notify_one();
	}
}
