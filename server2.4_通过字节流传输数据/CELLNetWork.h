#ifndef _CELL_NET_WORK_H_
#define _CELL_NET_WORK_H_
#include "CELL.h"

//��Ϊ�����������ж��ٸ�EasyServer���󣬶�����һ��
class CELLNetWork
{
private:
	CELLNetWork();
	/*
		��̬������ȫ������ֻ�е������˳���ʱ�򣬲Żᱻ�ͷţ��ڹ����ʱ��
		������һ��ȫ�ֵľ�̬����֮������������ٸ�EasyServer����
		CELLNetWork�Ĺ��캯�����ٱ�����
		�������˳���ʱ��������������һ�Σ����绷�������
	
	*/
	~CELLNetWork();

public:
	//���÷��ض�����ΪֻҪ������Ĺ��캯�������绷�����ܳ�ʼ��
	static void Init();
};

#endif // _CELL_NET_WORK_H_