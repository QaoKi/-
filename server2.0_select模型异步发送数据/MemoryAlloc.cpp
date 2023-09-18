#include "MemoryAlloc.h"
#include <memory.h>

MemoryAlloc::MemoryAlloc(int nSize, int nBlockCount)
{
	_pBuff = nullptr;
	_pHeader = nullptr;
	//��nSize���д���ʹ�������ϵͳ���ֽڶ��룬32λ��4�ֽڶ��룬64λ��8�ֽ�
	//ָ����32λ����4�ֽڣ�64λ����8�ֽ�
	//���磬nSize������61,�����8�ֽڶ��룬��ҪתΪ64
	const size_t n = sizeof(void*);
	size_t size = (nSize / n) * n + (nSize % n != 0 ? n : 0);
	_nBlockCount = nBlockCount;
	_nBufSize = size + sizeof(MemoryBlock);
}

MemoryAlloc::~MemoryAlloc()
{
	//���ڴ���ͷţ�ֻ�ͷ����ڴ�ص��ڴ棬û���ͷ���ϵͳ������ڴ�
	if (_pBuff)
		free(_pBuff); 
}

void MemoryAlloc::initMemory()
{
	//��ֹ�ظ�����
	if (_pBuff)
		return;

		
	//��ϵͳ����ص��ڴ�
	_pBuff = (char*)malloc(_nBufSize * _nBlockCount);

	memset(_pBuff, 0, _nBufSize * _nBlockCount);
	//��ʼ���ڴ�ص�ͷ���ڴ��
	_pHeader = (MemoryBlock*)_pBuff;
	_pHeader->bPool = true;
	_pHeader->nID = 0;
	_pHeader->nRef = 0;
	_pHeader->pAlloc = this;
	_pHeader->pNext = nullptr;

	//��ʼ���ڴ�����������ڴ��
	MemoryBlock* next = _pHeader;
	for (size_t n = 1; n < _nBlockCount; n++)
	{
		MemoryBlock* temp = (MemoryBlock*)(_pBuff + n * _nBufSize);
		temp->bPool = true;
		temp->nID = n;
		temp->nRef = 0;
		temp->pAlloc = this;
		next->pNext = temp;
		next = temp;
	}
}

void* MemoryAlloc::allocMemory(size_t size)
{
	std::lock_guard<std::mutex> lg(_mutex);
	if (!_pBuff)
		initMemory();

	MemoryBlock* pReturn = nullptr;
	//���ڴ����û���ڴ���ˣ��û�������ʱ��ֱ����ϵͳ����,�������ڴ浥Ԫͷ��Ϣ
	if (_pHeader == nullptr)
	{
		pReturn = (MemoryBlock*)malloc(_nBufSize + sizeof(MemoryBlock));
		memset(pReturn, 0, _nBufSize + sizeof(MemoryBlock));
		pReturn->bPool = false;
		pReturn->nID = -1;
		pReturn->nRef = 1;
		pReturn->pAlloc = this;
		pReturn->pNext = nullptr;
	}
	else
	{
		//�ڴ�������ڴ�飬ֱ�ӷ���
		pReturn = _pHeader;
		pReturn->nRef = 1;
		_pHeader = _pHeader->pNext;
	}
	xPrintf("allocMem:%lx,id = %d,size = %d\n", pReturn, pReturn->nID, size);
	//ֻ���û�����ʵ�ʿ����ڴ棬�������ڴ浥Ԫ�鲿��
	return ((char*)pReturn + sizeof(MemoryBlock));
}

void MemoryAlloc::freeMemory(void* pMem)
{ 
	if (!pMem)
		return;
	//Ҫȡ������ڴ���ڴ浥Ԫ�飬����֪������ϵͳ�Ļ����ڴ�����
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));

	if (--pBlock->nRef != 0)
	{
		//���������
		return;
	}
	if (pBlock->bPool)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		pBlock->pNext = _pHeader;
		_pHeader = pBlock;
	}
	else
	{
		//���ڳ��ڣ�ֱ��free
		free(pBlock);
	}
}
