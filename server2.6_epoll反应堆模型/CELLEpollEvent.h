#ifndef _CELL_EPOLL_EVENT_
#define _CELL_EPOLL_EVENT_
#include "CELL.h"
#include <vector>
#include <time.h>
#include <map>
#include <functional>
using namespace std;
typedef std::function<int(int,int)> EventCallBack;
class CELLEpollEvent
{
	struct CellEvent_s
	{
		int sock;			//Ҫ������sockst
		int events;			//Ҫ�������¼�
		//���ص������Ĳ�����������п��ޣ���Ϊ�ص������ǰ���ĳ�Ա��������ȥ����ֱ�Ӳ�����ĳ�Ա������
		//����ص���������ͨ�����Ļ���������this�Ѷ��󴫽�ȥ���ڻص������У�arg��תΪ����
		void* arg;			
		//std::function���bind�����Խ���ĳ�Ա������Ϊ�ص�����������function�Ĳ����в��ܴ�void*
		//���Ҫ��void*������ typedef (*EventCallBack)(int,int,void*)
		EventCallBack callback;
		int status;			//�Ƿ��ڼ�����1->�ں������(����)��0->���ڣ���������
		int buf[1024];
		int len;			//buf�ĳ���
		long last_active;	//��¼ÿ�μ���������ʱ��ֵ���������
	};
public:
	CELLEpollEvent(int nSize);
	~CELLEpollEvent();

	void eventAdd(int sock, void *arg);

	void eventSet(int sock, int events,EventCallBack callback);

	void eventDel(int sock);

	int eventStart();

private:
	int _epSock;		//����epoll_create()���������ĺ��������
	map<SOCKET, CellEvent_s*> _mapCellEvent;
};
#endif // _CELL_EPOLL_EVENT_
