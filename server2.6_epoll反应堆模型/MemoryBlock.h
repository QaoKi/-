#ifndef _MemoryBlock_H
#define _MemoryBlock_H
//#include "MemoryAlloc.h"

class MemoryAlloc;
//�ڴ��  ��С��Ԫ
//64Ϊ��8�ֽڶ��룬32Ϊ��4�ֽڶ���
class MemoryBlock
{
public:
	MemoryBlock();
	~MemoryBlock();

public:
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�������ڴ�飨�أ�
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�Ƿ����ڴ����(���ڴ���еĿ������ˣ��û������룬ֱ����ϵͳ����)
	bool bPool;
private:
	//Ԥ��
	//char cNULL;
};

//int MemoryBlockSize = sizeof(MemoryBlock);
#endif // 

