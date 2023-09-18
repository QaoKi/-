#ifndef _MemoryAlloc_H
#define _MemoryAlloc_H
#include <stdlib.h>
//#include "MemoryBlock.h"

class MemoryBlock;
//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc();
	~MemoryAlloc();

	//��ʼ���ڴ��
	//�����ڴ�
	void initMemory();
	void* allocMemory(size_t size);
	//�ͷ��ڴ�
	void freeMemory(void* p);

private:
	char* applicationMemory();
private:
	//�ڴ�ص�ַ
	char* m_pBuff;
	//ͷ���ڴ浥Ԫ,���õĵ�һ����
	MemoryBlock* m_pHeader;
	//ÿ���ڴ浥Ԫ�Ĵ�С�������˵�Ԫ��Ϣ��
	size_t m_nSize;
	//�ڴ浥Ԫ������
	size_t m_nBlockSize;

};

#endif

