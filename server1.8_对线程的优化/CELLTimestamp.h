#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include<chrono>
using namespace std::chrono;

//�õ�һ��ʱ����
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
	time_point<high_resolution_clock> _begin;

};

//�õ���ǰʱ��
class CELLTime
{

public:
	//�õ���ǰʱ�䣬ת��Ϊ����
	static time_t getNowTimeInMilliSec();

};


#endif
