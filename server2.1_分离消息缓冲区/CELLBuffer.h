#ifndef _CELL_BUFFER_H_
#define _CELL_BUFFER_H_

#ifdef _WIN32
#include<windows.h>
#else
#include<sys/socket.h>
#include<unistd.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <memory>
#include "MessageHeader.hpp"

class CELLBuffer
{
public:
	//������Ĭ����8k�Ĵ�С
	CELLBuffer(int nSize = 8192);
	~CELLBuffer();

	//���ػ�����
	char* getData();

	//�򻺳�����������
	bool push(const char* pData,int nLen);

	//��ͷɾ������������
	void pop(int nLen);

	//�����������ݷ��͵�socket
	int write2socket(SOCKET sockfd);
	
	//��socket�ж�ȡ���ݶ�������
	int read4socket(SOCKET sockfd);

	//�жϻ������е����ݳ����Ƿ�һ����Ϣ�ĳ���
	bool hasMag();

private:
	//������
	char* _pBuff = nullptr;
	//������Ҳ�������������������������ݿ�
	//list<char*> _pBuffList;
	//������������β��λ�ã��������ݳ���
	int _nLast = 0;
	//�������ܵĿռ��С���ֽڳ���
	int _nSize = 0;

	//������д��������������Ҫ���ڵ������ͻ�����ʱʹ��
	int _nFullCount = 0;

};
#endif

