#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_

#include <stdlib.h>
#include <mutex>
#include "Alloctor.h"

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

//����ؽڵ���Ϣ
class NodeHeader
{
public:
	//��һ��λ��
	NodeHeader* pNext;
	//�ڴ����
	int nID;
	//���ô���
	char nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	//char c1;
};

template<class Type,size_t nPoolSize>
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		m_nBufSize = sizeof(Type) + sizeof(NodeHeader);
		initPool();
	}
	~CELLObjectPool()
	{
		if (m_pBuff)
			delete[] m_pBuff;
	}
public:

	//�������
	void* allocObj(size_t size)
	{
		std::lock_guard<std::mutex> lg(m_mutex);
		if (!m_pBuff)
			initPool();

		NodeHeader* pReturn = nullptr;
		//���������û�ж����ˣ��û�������ʱ��ֱ�����ڴ������,�����϶���Ԫͷ��Ϣ
		if (m_pHeader == nullptr)
		{
			pReturn = (NodeHeader*)new char[m_nBufSize];
			memset(pReturn, 0, m_nBufSize);
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			//��������ж���ֱ�ӷ���
			pReturn = m_pHeader;
			pReturn->nRef = 1;
			m_pHeader = m_pHeader->pNext;
		}
		xPrintf("allocObj: %lx,id = %d,size = %d\n", pReturn, pReturn->nID, size);
		//ֻ���û�����ʵ�ʿ����ڴ棬�������ڴ浥Ԫ�鲿��
		return ((char*)pReturn + sizeof(NodeHeader));
	}
	//�ͷŶ���
	void freeObj(void* pMem)
	{
		if (!pMem)
			return;
		//Ҫȡ���������Ķ�����Ϣ��Ԫ�飬����֪�����Ƿ��Ƕ�����е�
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));

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
			//���ڳ��ڣ�ֱ��delete
			delete[] pBlock;
		}
		
	}
private:
	//��ʼ�������
	void initPool()
	{
		if (m_pBuff)
			return;
		//�������صĴ�С
		size_t size = nPoolSize * m_nBufSize;
		//���ڴ�������ڴ�
		m_pBuff = new char[size];

		//��ʼ������ص�ͷ��
		m_pHeader = (NodeHeader*)m_pBuff;
		m_pHeader->bPool = true;
		m_pHeader->nID = 0;
		m_pHeader->nRef = 0;
		m_pHeader->pNext = nullptr;

		//��ʼ��������е�����ڵ�
		NodeHeader* next = m_pHeader;
		for (size_t n = 1; n < nPoolSize; n++)
		{
			NodeHeader* temp = (NodeHeader*)(m_pBuff + n * m_nBufSize);
			temp->bPool = true;
			temp->nID = n;
			temp->nRef = 0;
			temp->pNext = nullptr;
			next->pNext = temp;
			next = temp;
		}
	}
	//ͷ��
	NodeHeader* m_pHeader;
	//������ڴ滺������ַ
	char* m_pBuff;
	//ÿ������Ĵ�С������NodeHeader
	size_t m_nBufSize;
	std::mutex m_mutex;
	
};

//Ϊ�˸�ÿ���ඨ��һ��ͨ�õ�createObject()��destroyObject()
//createObject()�п�����һЩ��������

template<class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:

	void* operator new(size_t nSize)
	{
		return ObjectPoolBase::objectPool().allocObj(nSize);
	}

	void operator delete(void* p)
	{
		ObjectPoolBase::objectPool().freeObj(p);
	}

	//����һ�����ڴ�����������ٶ���ĺ���
	template<typename ...Args>		//�ɱ����
	static Type* createObject(Args ... args)
	{
		Type* obj = new Type(args...);
		//��Щ��������
		return obj;
	}

	static void destroyObject(Type* obj)
	{
		delete obj;
	}

private:
	//ͬһ���࣬ʹ��ͬһ�������
	typedef CELLObjectPool<Type,nPoolSize> ClassTypePool;
	static ClassTypePool& objectPool()
	{
		//��̬����ض���
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif // _ObjectPoolBase_H_


