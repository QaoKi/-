#ifndef _MemoryAlloc_H
#define _MemoryAlloc_H
#include <stdlib.h>
#include "MemoryBlock.h"
#include<mutex>//��

//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc(int nSize, int nBlockCount);
	~MemoryAlloc();

	//��ʼ���ڴ��
	//�����ڴ�
	void initMemory();
	void* allocMemory(size_t size);
	//�ͷ��ڴ�
	void freeMemory(void* p);
private:
	//�ڴ�ص�ַ
	char* _pBuff;
	//ͷ���ڴ浥Ԫ,���õĵ�һ����
	MemoryBlock* _pHeader;
	//ÿ���ڴ浥Ԫ�Ĵ�С�������˵�Ԫ��Ϣ��
	size_t _nBufSize;
	//�ڴ浥Ԫ������
	size_t _nBlockCount;

	std::mutex _mutex;
};

#endif

