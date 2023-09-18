#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_

#include <stdlib.h>
#include <mutex>
#include <memory.h>
#include "Alloctor.h"

//��ӡ������Ϣ��ֻ����debugģʽ�²Ż����
#ifdef _DEBUG
#ifndef xPrintf
	#include <stdio.h>
	//(...) ��ʾ��������� 
	#define xPrintf Printf(__VA_ARGS__)
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
	NodeHeader()
	{
		pNext = nullptr;
	}
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
		_pBuff = nullptr;
		_nBufSize = sizeof(Type) + sizeof(NodeHeader);
		initPool();
	}
	~CELLObjectPool()
	{
		if (_pBuff)
			delete[] _pBuff;
	}
public:

	//�������
	void* allocObj(size_t size)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuff)
			initPool();

		NodeHeader* pReturn = nullptr;
		//���������û�ж����ˣ��û�������ʱ��ֱ�����ڴ������,�����϶���Ԫͷ��Ϣ
		if (_pHeader == nullptr)
		{
			pReturn = (NodeHeader*)new char[_nBufSize];
			memset(pReturn, 0, _nBufSize);
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			//��������ж���ֱ�ӷ���
			pReturn = _pHeader;
			pReturn->nRef = 1;
			_pHeader = _pHeader->pNext;
		}
		xPrintf("allocObj:%lx,id = %d,size = %d\n", pReturn, pReturn->nID, size);
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
			std::lock_guard<std::mutex> lg(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
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
		if (_pBuff)
			return;
		//�������صĴ�С
		size_t size = nPoolSize * _nBufSize;
		//���ڴ�������ڴ�
		_pBuff = new char[size];

		//��ʼ������ص�ͷ��
		_pHeader = (NodeHeader*)_pBuff;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;

		//��ʼ��������е�����ڵ�
		NodeHeader* next = _pHeader;
		for (size_t n = 1; n < nPoolSize; n++)
		{
			NodeHeader* temp = (NodeHeader*)(_pBuff + n * _nBufSize);
			temp->bPool = true;
			temp->nID = n;
			temp->nRef = 0;
			temp->pNext = nullptr;
			next->pNext = temp;
			next = temp;
		}
	}
	//ͷ��
	NodeHeader* _pHeader;
	//������ڴ滺������ַ
	char* _pBuff;
	//ÿ������Ĵ�С������NodeHeader
	size_t _nBufSize;
	std::mutex _mutex;
	
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


