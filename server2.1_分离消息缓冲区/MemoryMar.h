#ifndef _MemoryMar_H
#define _MemoryMar_H
#include <stdlib.h>
#include "MemoryAlloc.h"

//�ڴ������

//��ӡ������Ϣ��ֻ����debugģʽ�²Ż����
#ifdef _DEBUG
#include <stdio.h>
//(...) ��ʾ��������� 
	#define xPrintf(...) Printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif



//����ڴ浥Ԫ
#define MAX_MEMORY_SIZE 1024

class MemoryMar
{
private:
	MemoryMar();
	~MemoryMar();

public:
	static MemoryMar& Instance();
	//�����ڴ�
	void* allocMem(size_t size);
	//�ͷ��ڴ�
	void freeMem(void* p);
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA);

	MemoryAlloc mem64{64,100000};
	MemoryAlloc mem128{ 128,100000 };
	MemoryAlloc mem256{ 256,100000 };
	MemoryAlloc mem512{ 512,100000 };
	MemoryAlloc mem1024{ 1024,100000 };
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};

#endif
