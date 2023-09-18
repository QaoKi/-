#ifndef _CELL_LOG_H_
#define _CELL_LOG_H_
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include "CellTaskServer.h"
//������־����Ϊ����ģʽ

/*
	ʹ�ò���
	��ʹ��֮ǰ������ CELLLog::Instance().setLogPath("serverLog.txt", "w");����·�����ļ��򿪷�ʽ
	CELLLog::Info("ֱ�����");
	int n = 10;
	const char* str = "n = %d";
	CELLLog::Info(str,n);

*/
class CELLLog
{
private:
	CELLLog();
	~CELLLog();

public:

	static CELLLog& Instance();

	void setLogPath(const char* logpath,const char* mode);

	//������Ϣ������ַ���
	static void Info(const char* pStr);
	//�ɱ����
	template<typename ...Args>
	static void Info(const char* pformat, Args ...args);
	//������Ϣ
	//������Ϣ
	//������Ϣ
private:
	FILE* _logFile;
	CellTaskServer taskServer;
};



#endif // _CELL_LOG_H_
