#ifndef _Alloctor_H
#define _Alloctor_H
#include "MemoryMar.h"

/*
	Allcotor.h��Allcotor.cpp�����࣬�����޷�����һ��MemoryMar��ʵ��������ʹ��
	������ҪMemoryMar��Ϊ����
*/

void* operator new(size_t size);

void operator delete(void* p);

void* operator new[](size_t size);

void operator delete[](void* p);

void* me_alloc(size_t size);

void me_free(void* p);

#endif // !_Alloctor_H
