#include "MemoryAlloc.h"
#include <memory.h>

MemoryAlloc::MemoryAlloc(int nSize, int nBlockCount)
{
	m_pBuff = nullptr;
	m_pHeader = nullptr;
	//��nSize���д���ʹ�������ϵͳ���ֽڶ��룬32λ��4�ֽڶ��룬64λ��8�ֽ�
	//ָ����32λ����4�ֽڣ�64λ����8�ֽ�
	//���磬nSize������61,�����8�ֽڶ��룬��ҪתΪ64
	const size_t n = sizeof(void*);
	size_t size = (nSize / n) * n + (nSize % n != 0 ? n : 0);
	m_nBlockCount = nBlockCount;
	m_nBufSize = size + sizeof(MemoryBlock);
}

MemoryAlloc::~MemoryAlloc()
{
	//���ڴ���ͷţ�ֻ�ͷ����ڴ�ص��ڴ棬û���ͷ���ϵͳ������ڴ�
	if (m_pBuff)
		free(m_pBuff); 
}

void MemoryAlloc::initMemory()
{
	//��ֹ�ظ�����
	if (m_pBuff)
		return;
	
	//��ϵͳ����ص��ڴ�
	m_pBuff = (char*)malloc(m_nBufSize * m_nBlockCount);

	memset(m_pBuff, 0, m_nBufSize * m_nBlockCount);
	//��ʼ���ڴ�ص�ͷ���ڴ��
	m_pHeader = (MemoryBlock*)m_pBuff;
	m_pHeader->bPool = true;
	m_pHeader->nID = 0;
	m_pHeader->nRef = 0;
	m_pHeader->pAlloc = this;
	m_pHeader->pNext = nullptr;

	//��ʼ���ڴ�����������ڴ��
	MemoryBlock* next = m_pHeader;
	for (size_t n = 1; n < m_nBlockCount; n++)
	{
		MemoryBlock* temp = (MemoryBlock*)(m_pBuff + n * m_nBufSize);
		temp->bPool = true;
		temp->nID = n;
		temp->nRef = 0;
		temp->pAlloc = this;
		temp->pNext = nullptr;
		next->pNext = temp;
		next = temp;
	}
}

void* MemoryAlloc::allocMemory(size_t size)
{
	std::lock_guard<std::mutex> lg(m_mutex);
	if (!m_pBuff)
		initMemory();

	MemoryBlock* pReturn = nullptr;
	//���ڴ����û���ڴ���ˣ��û�������ʱ��ֱ����ϵͳ����,�������ڴ浥Ԫͷ��Ϣ
	if (m_pHeader == nullptr)
	{
		pReturn = (MemoryBlock*)malloc(m_nBufSize);
		memset(pReturn, 0, m_nBufSize);
		pReturn->bPool = false;
		pReturn->nID = -1;
		pReturn->nRef = 1;
		pReturn->pAlloc = this;
		pReturn->pNext = nullptr;
	}
	else
	{
		//�ڴ�������ڴ�飬ֱ�ӷ���
		pReturn = m_pHeader;
		pReturn->nRef = 1;
		m_pHeader = m_pHeader->pNext;
	}
	xPrintf("allocMem: %lx,id = %d,size = %d\n", pReturn, pReturn->nID, size);
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
		std::lock_guard<std::mutex> lg(m_mutex);
		pBlock->pNext = m_pHeader;
		m_pHeader = pBlock;
	}
	else
	{
		//���ڳ��ڣ�ֱ��free
		free(pBlock);
	}
}
