#ifndef _CellTask_H
#define _CellTask_H

#include <list>
#include <mutex>
#include <thread>
#include "ClientSocket.h"

//�������ʹ��
using namespace std;

class CellTask;
typedef std::shared_ptr<CellTask> CellTaskPtr;
//�������͵Ļ���
class CellTask
{
public:
	CellTask();
	virtual ~CellTask();

	virtual void doTask();
};

#endif
