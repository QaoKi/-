#include "CELLBuffer.h"



CELLBuffer::CELLBuffer(int nSize /*= 8192*/)
{
	_nSize = nSize;
	_pBuff = new char[_nSize];
}

CELLBuffer::~CELLBuffer()
{
	if (_pBuff)
	{
		delete[] _pBuff;
		_pBuff = nullptr;
	}
}

char* CELLBuffer::getData()
{
	return _pBuff;
}

bool CELLBuffer::push(const char* pData, int nLen)
{
	if (!pData || nLen <= 0)
	{
		return false;
	}
	/*
		�����ṩ��һ��������������ʱ�Ľ����������д���������ʱ��
		Ҳ����д�����ݿ���ߴ�����
	*/
	//��Ҫ��������ݳ��ȣ����ڻ���������ĳ���,��̬��չ
	if (nLen > _nSize - _nLast)
	{
		//�ռ�����
		int len = _nLast + nLen - _nSize;
		//��Ҫ��չ����_nSize�ĳ���
		int num = len / _nSize + 1;

		//ԭ�������ݻ���Ҫһ��
		char* buff = new char[(num + 1)*_nSize];
		memcpy(buff, _pBuff, _nLast);
		_nSize = (num + 1)*_nSize;
		delete[] _pBuff;
		_pBuff = buff;
	}

	//��Ҫ��������ݳ��ȣ�С�ڵ��ڻ���������ĳ��ȣ������ݷŵ�������
	if (nLen <= _nSize - _nLast)
	{
		memcpy(_pBuff + _nLast, pData, nLen);
		_nLast += nLen;
		if (_nLast == _nSize)
		{
			_nFullCount++;
		}
		return true;
	}
	else
	{
		_nFullCount++;
	}
	return false;
}

void CELLBuffer::pop(int nLen)
{
	//nLen��Ҫ�Ƴ��ĳ���
	int n = _nLast - nLen;
	if (n > 0)
	{
		//����������ݣ����ǵ�ǰ��
		memcpy(_pBuff, _pBuff + nLen, n);
		_nLast = n;
	}
	else
	{
		memset(_pBuff, 0, sizeof(_pBuff));
		_nLast = 0;
	}
	if (_nFullCount > 0)
		--_nFullCount;
}

int CELLBuffer::write2socket(SOCKET sockfd)
{
	int nRet = SOCKET_ERROR;
	//������������
	//��_nLast == 0 ˵����������û�����ݣ�������󣬷���0

	if (_nLast == 0)
	{
		nRet = 0;
	}
	else if (_nLast > 0 && INVALID_SOCKET != sockfd)
	{
		nRet = send(sockfd, _pBuff, _nLast, 0);

		if (nRet > 0)
		{
			//���û�����ָ��λ��
			_nLast = 0;
			//���û�����������
			_nFullCount = 0;
		}
		else
		{
			//��Ϊsend()�ķ���ֵ����Ϊ0��send()����0��ʾ��������Ҫ����һ��
			nRet = SOCKET_ERROR;
		}
	}
	return nRet;
}

int CELLBuffer::read4socket(SOCKET sockfd)
{
	//�п�λ�ò�ȥ��������
	if (_nSize - _nLast > 0)
	{
		int nLen = recv(sockfd, _pBuff + _nLast, _nSize - _nLast, 0);
		//recv����<=0����󣬷���-1
		if (nLen <= 0)
		{
			return SOCKET_ERROR;
		}
		//������������β��λ�ú���
		_nLast += nLen;
	}
	//������û�п��У�����0���������
	return 0;
}

bool CELLBuffer::hasMag()
{
	//��Ϣ�������е����ݣ������Ƿ������ϢͷDataHeader����
	if (_nLast >= sizeof(netmsg_DataHeader))
	{
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
		//��Ϣ�������е����ݳ��ȣ��Ƿ���ڵ��ڵ�ǰ��Ϣ�ĳ���
		//������ڵ��ڣ�˵���������е����ݹ�һ����Ϣ��
		return _nLast >= header->dataLength;
	}
	return false;
}
