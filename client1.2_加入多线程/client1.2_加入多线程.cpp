#include<iostream>
#include "EasyClient.h"
#include <thread>

using namespace std;

//�ͻ�������
const int cCount = 1;
//�����߳�����
const int tCount = 1;
//�ͻ�������
EasyClient* client[cCount];


void sendThread(int id)
{
	//4���̣߳�id 1~4,1000���ͻ��ˣ�ÿ���̸߳���250��
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;

	for (int i = begin; i < end; i++)
	{
		client[i] = new EasyClient();
	}

	for (int i = begin; i < end; i++)
	{
		client[i]->Connect("192.168.1.106", 8081);
		client[i]->Start();
		//printf("thread<%d>,Connect=%d\n", id, i);
	}

	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].userName, "lyd");
		strcpy(login[i].PassWord, "lydmm");
	}

	const int nLen = sizeof(login);
	bool isSend = true;
	while (true)
	{
		for (int i = begin; i < end; i++)
		{
			//if (isSend)
			//{
			client[i]->SendCmd("login");
			//client[i]->SendData(login, nLen);
			//client[i]->Run();
			//isSend = false;
			//}
		}
		chrono::milliseconds t(1000);
		this_thread::sleep_for(t);
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}

}
int main()
{

	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(sendThread,i + 1);	//���ǵڼ����̴߳���ȥ
		t1.detach();	//detach()�����̺߳����̷߳��룬���̲߳���ȴ��̵߳Ľ���
		//t1.join();	//join()���߳�ִ�����Ժ����̲߳Ż��������ִ�У���������˺ܶ��̣߳���Щ�߳�֮���ǲ��е�
	}

	while (true)
	{
		chrono::milliseconds t(100);
		this_thread::sleep_for(t);
	}
	system("pause");
	return 0;
}