#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include<chrono>
using namespace std::chrono;
class CELLTimestamp
{
public:
	CELLTimestamp();
	~CELLTimestamp();

public:
	//��¼��ǰʱ��
	void update();

	//��ȡ��
	double getElapsedSecond();

	//��ȡ����
	double getElapsedTimeInMilliSec();

	//��ȡ΢��
	long long GetElapsedTimeInMicroSec();

private:
	//����һ��ʱ���
	time_point<high_resolution_clock> m_begin;

};

#endif
