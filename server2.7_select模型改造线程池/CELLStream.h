#ifndef _CELL_STREAM_H_
#define _CELL_STREAM_H_
#include <cstdint>
#include <memory>
//�ֽ���
class CELLStream
{
public:
	/*
	���ִ������ݵ��ֽ����ķ�ʽ��һ�����ֶ�����һ���ڴ棬Ȼ��д��
	һ�����������ⲿ������ڴ棬ֱ�Ӱ�����д�룬�������ֽ���������ڴ��Ƿ���Ҫ�ֽ������ͷ�
	*/
	//ֱ�Ӵ������ݣ��������ж��
	CELLStream(char* pData, int nSize, bool bDelete = false);
	//�ֶ������ڴ�д������
	CELLStream(int nSize = 1024);
	virtual ~CELLStream();
public:
	char* getData();
	//�ڲ�ͬ�Ĳ���ϵͳ��ƽ̨���棬�����������͵Ĵ�С�ǲ�һ����
	//int8_t�� 8λ������8λ��һ���ֽ�
	//int16_t�� 16λ������16λ�������ֽڣ�int32_t���ĸ��ֽڣ������͹̶��˴�С

	//�ֽ���д�������Ժ�_nWritePos��Ҫ����n
	//inline�����������Ͷ���Ҳд��һ��
	inline void push(int n)
	{
		_nWritePos += n;
	}

	//�ֽ������������Ժ�_nReadPos��Ҫ����n
	inline void pop(int n)
	{
		_nReadPos += n;
	}

	inline void setWritePos(int n)
	{
		_nWritePos = n;
	}
	inline int getWritePos()
	{
		return _nWritePos;
	}

	//read�����ֽ����ж�����
	int8_t ReadInt8();	
	int16_t ReadInt16();
	int32_t ReadInt32();
	float ReadFloat();
	double ReadDouble();
	//��ȡ��������
	template<typename T>
	uint32_t ReadArray(T* pArr, uint32_t len)
	{
		//����len�ǻ��������ܴ�Ŷ��ٸ�Ԫ��
		//�ȶ�ȡ����Ԫ�ظ���
		uint32_t num = 0;
		//��_nReadPos�Ȳ�Ҫ���ƫ�ƣ���Ϊ��ȷ���������Ƿ��ܳɹ�
		//���ֱ��ƫ���ˣ�����Ԫ�ظ�����û���ٴζ���
		OnlyRead(num);
		//�жϻ��������ڴ��Ƿ�
		if (len > num)
		{
			//Ҫȡ�����ݵ��ֽڳ���
			auto nLen = num * sizeof(T);
			//�ж��ܲ��ܶ���
			//��ʱ_nReadPosָ������Ԫ�ظ��������ƫ��sizeof(uint32_t)�������ݵĿ�ʼ
			if (canRead(nLen + sizeof(uint32_t)))
			{
				//�����ȷ���ܶ�ȡ���ݣ�����_nReadPosƫ��
				pop(sizeof(uint32_t));
				memcpy(pArr, _pBuff + _nReadPos, nLen);
				pop(nLen);
				//����ʵ�ʶ�ȡ��Ԫ�صĸ���
				return num;
			}
		}
	}


	//write�����ֽ�����д����
	//д��8λ�������൱��д��1���ֽڣ���Ӧchar���ͣ������Ƿ�ɹ�
	bool WriteInt8(int8_t n);
	//д��16λ�������൱��д��2���ֽڣ���Ӧshort���ͣ������Ƿ�ɹ�
	bool WriteInt16(int16_t n);
	//д��32λ�������൱��д��4���ֽڣ���Ӧint���ͣ������Ƿ�ɹ�
	bool WriteInt32(int32_t n);
	//д��float����
	bool WriteFloat(float n);
	//д��double����
	bool WriteDouble(double n);

	/*
		д�����鲻��ҪΪÿһ������,����int��char�ٶ��ⶨ�巽������Ϊ������
		�����ʱ�������Ѿ�ȷ���ˣ����Ե���WriteArray��ʱ�����ʽת��
		������Ļ�������֮����Ҫ�ض�����ô�෽��
		��Ϊ���紫��Write(5),��ʱ��Ĭ��Ϊint���ͣ���4���ֽ�
		��WriteInt8(5)������Ϊ1���ֽڴ洢	
	*/
	//��Ϊ��ģ�庯�������.h��.cpp�ֿ�������ģ�庯���������ʱ��ᱨ��
	//����ģ�庯����������ʵ�ַŵ�һ��
	template<typename T>
	bool WriteArray(T* pData, uint32_t len)
	{
		//����Ҫд���������ֽڳ���
		auto nLen = sizeof(int) * len;
		//��Ҫ��������ݳ��ȣ�С�ڵ��ڻ���������ĳ��ȣ������ݷŵ�������
		//��ΪҲҪ������Ԫ�ظ���д��ȥ������Ҫ��һ�� sizeof(uint32_t)
		if (canWrite(nLen + sizeof(len)))
		{
			//ֻд����������ݣ�����ȡ�����ʱ���޷�֪�������ж೤
			//����д����������֮ǰ���Ȱ������Ԫ�ظ���д�룬����len��uint32_t����
			//����ñ�����ͣ��ڸ���ϵͳ���泤���ֻ᲻һ��
			Write(len);
			memcpy(_pBuff + _nWritePos, pData, nLen);
			push(nLen);
			return true;
		}
		return false;
	}
public:

	//������ģ�壬���Ҷ����Ժ�_nReadPos����ƫ��
	//����ȡ�������ݵ�ʱ��Ҫ�ȶ�ȡ����Ԫ�صĸ�������ȡ���Ժ�_nReadPos������ƶ���
	//���������������û�гɹ�����ô�Ͳ����ٶ�������Ԫ�ظ����ˣ�
	//���Լ�һ��ֻ�����ݣ�����_nReadPosƫ�Ƶķ���OnlyRead()
	template<typename T>
	T Read(T& n)
	{
		//Ҫ��ȡ�����ݳ���
		auto nLen = sizeof(T);
		//��û������
		if (canRead(nLen))
		{
			//��Ҫ��ȡ�����ݣ���������
			memcpy(&n, _pBuff + _nReadPos, nLen);
			pop(nLen);
			return n;
		}
		return -1;
	}

	template<typename T>
	T OnlyRead(T& n)
	{
		//Ҫ��ȡ�����ݳ���
		auto nLen = sizeof(T);
		//��û������
		if (canRead(nLen))
		{
			//��Ҫ��ȡ�����ݣ���������������_nReadPosƫ��
			memcpy(&n, _pBuff + _nReadPos, nLen);
			return n;
		}
		return -1;
	}

	//д��������ݵ�ģ�庯��
	template<typename T>
	bool Write(T n)
	{
		auto nlen = sizeof(T);
		//��Ҫ��������ݳ��ȣ�С�ڵ��ڻ���������ĳ��ȣ������ݷŵ�������
		if (canWrite(nlen))
		{
			memcpy(_pBuff + _nWritePos, &n, nlen);
			push(nlen);
			return true;
		}
		return false;
	}

private:

	//�ж��ܷ���ж�д
	inline bool canRead(int n);
	inline bool canWrite(int n);

	//���ݻ�����
	char* _pBuff = nullptr;
	//�������ܵĿռ��С���ֽڳ���
	int _nSize = 0;
	//���е�����β��λ�ã����е����ݳ���
	int _nWritePos = 0;
	//�Ѷ�ȡ���ݵ�β��λ�ã������￪ʼ��
	int _nReadPos = 0;
	//��_pBuff���ⲿ���������ʱ���Ƿ�Ӧ�����ֽ����ͷ�
	bool _bDelete = true;
};


#endif // _CELL_STREAM_H_
