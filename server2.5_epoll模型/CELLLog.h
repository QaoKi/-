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
	static void Info(const char* pformat, Args ... args)
	{

		auto pLog = &Instance();
		pLog->taskServer.AddTask([pLog, pformat, args...]() {
			if (pLog->_logFile)
			{
				time_t tTemp;
				time(&tTemp);
				struct tm* tNow = gmtime(&tTemp);
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, tNow->tm_hour + 8, tNow->tm_min, tNow->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				//ʵʱд��
				fflush(pLog->_logFile);
			}
			printf(pformat, args...);
		});	
	}
	//������Ϣ
	//������Ϣ
	//������Ϣ
private:
	FILE* _logFile;
	CellTaskServer taskServer;
};



#endif // _CELL_LOG_H_
