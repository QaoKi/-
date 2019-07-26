#include "CELLTimestamp.h"



CELLTimestamp::CELLTimestamp()
{
	//��ʼ��ʱ��¼��ǰʱ��
	update();
}


CELLTimestamp::~CELLTimestamp()
{
}

void CELLTimestamp::update()
{
	//high_resolution_clock::now() ���߾��ȵĻ�ȡ��ǰʱ�䣬��ȷ��΢��
	m_begin = high_resolution_clock::now();
}

double CELLTimestamp::getElapsedSecond()
{
	return GetElapsedTimeInMicroSec() *  0.000001;
}

double CELLTimestamp::getElapsedTimeInMilliSec()
{
	return GetElapsedTimeInMicroSec() *  0.001;
}

long long CELLTimestamp::GetElapsedTimeInMicroSec()
{
	//�õ�ǰʱ�䣬��ȥ֮ǰ��¼��ʱ�䣬�������ʱ��ļ����������΢���long long�洢
	//microseconds��΢�milliseconds�Ǻ���,seconds����
	return duration_cast<microseconds>(high_resolution_clock::now() - m_begin).count();
}
