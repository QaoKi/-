#include "MemoryAlloc.h"
#include "MemoryBlock.h"

MemoryAlloc::MemoryAlloc()
{
	m_pBuff = nullptr;
	m_pHeader = nullptr;
	m_nSize = 0;
	m_nBlockSize = 0;
}


MemoryAlloc::~MemoryAlloc()
{
	//���ڴ���ͷ�
	if (m_pBuff)
		free(m_pBuff); 
}

void MemoryAlloc::initMemory()
{
	//��ֹ�ظ�����
	if (m_pBuff)
		return;
	//��ϵͳ����ص��ڴ�
	m_pBuff = applicationMemory();
}

void* MemoryAlloc::allocMemory(size_t size)
{
	if (!m_pBuff)
		initMemory();

	MemoryBlock* pReturn = nullptr;
	//���ڴ����û���ڴ���ˣ��û�������ʱ���ڴ�ض�̬����
	if (m_pHeader == nullptr)
	{
		pReturn = (MemoryBlock*)malloc(m_nSize + sizeof(MemoryBlock));
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
		pBlock->pNext = m_pHeader;
		m_pHeader = pBlock;
	}
	else
	{
		//���ڳ��ڣ�ֱ��free
		free(pBlock);
	}
}

char* MemoryAlloc::applicationMemory()
{
	//��ϵͳ����ص��ڴ�
	char* pReturn = (char*)malloc(m_nSize * m_nBlockSize);

	//��ʼ���ڴ�ص�ͷ���ڴ��
	m_pHeader = (MemoryBlock*)pReturn;
	m_pHeader->bPool = true;
	m_pHeader->nID = 0;
	m_pHeader->nRef = 0;
	m_pHeader->pAlloc = this;
	m_pHeader->pNext = nullptr;

	//��ʼ���ڴ�����������ڴ��
	MemoryBlock* next = m_pHeader;
	for (size_t n = 1; n < m_nBlockSize; n++)
	{
		MemoryBlock* temp = (MemoryBlock*)(pReturn + n * m_nSize);
		temp->bPool = true;
		temp->nID = 0;
		temp->nRef = 0;
		temp->pAlloc = this;
		next->pNext = temp;
		next = temp;
	}

	return pReturn;
}
