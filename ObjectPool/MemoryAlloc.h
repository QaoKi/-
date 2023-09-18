#ifndef _MemoryAlloc_H
#define _MemoryAlloc_H
#include <stdlib.h>
#include "MemoryBlock.h"
#include<mutex>//��

//��ӡ������Ϣ��ֻ����debugģʽ�²Ż����
#ifdef _DEBUG
#ifndef xPrintf
#include <stdio.h>
//(...) ��ʾ��������� 
#define xPrintf(...) printf(__VA_ARGS__)
#endif
#else
#ifndef xPrintf
#define xPrintf(...)
#endif
#endif

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
	char* m_pBuff;
	//ͷ���ڴ浥Ԫ,���õĵ�һ����
	MemoryBlock* m_pHeader;
	//ÿ���ڴ浥Ԫ�Ĵ�С�������˵�Ԫ��Ϣ��
	size_t m_nBufSize;
	//�ڴ浥Ԫ������
	size_t m_nBlockCount;

	std::mutex m_mutex;
};

#endif

