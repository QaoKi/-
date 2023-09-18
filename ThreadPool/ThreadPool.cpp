#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
using namespace std;

#ifdef _WIN32
//��windows�±���
typedef unsigned long int pthread_t;	
typedef mutex pthread_mutex_t;
typedef condition_variable pthread_cond_t;

#endif


typedef struct {
	void* (*function)(void*);		//����ָ�룬�ص�����
	void* arg;						//�ص������Ĳ���	
}threadpool_task_t;

/*�����̳߳������Ϣ*/
struct threadpool_t
{
	pthread_mutex_t lock;				//������ס���ṹ��
	pthread_mutex_t thread_counter;		//���ڲ���busy_thr_numʱʹ��
	pthread_cond_t queue_full;		    //�����������ʱ�����������߳��������ȴ�����������
	pthread_cond_t queue_empty;			//�������Ϊ��ʱ�������߳�

	pthread_t* threads;					//����̳߳���ÿ���̵߳�tid������
	pthread_t  adjust_tid;				//��Ź������߳�tid�����ڼ�⵱ǰ�̳߳�ʹ�����
	threadpool_task_t* task_queue;		//�������

	int min_thr_num;					//�̳߳���С�߳���
	int max_thr_num;					//�̳߳�����߳���
	int live_thr_num;					//��ǰ����̸߳���
	int busy_thr_num;					//æ״̬�̸߳���
	int wait_exit_thr_num;				//Ҫ���ٵ��̸߳���

	int queue_front;					//task_queue��ͷ�±�
	int queue_rear;						//task_queue��β�±�
	int queue_size;						//task_queue����ʵ��������
	int queue_max_size;					//task_queue�����п���������������

	int shutdown;						//��־λ���̳߳�ʹ��״̬��true��false
};

/*
	�̳߳��и��������߳�
*/
void* threadpool_thread(void* arg)
{
	threadpool_t* pool = (threadpool_t*)arg;
	threadpool_task_t task;

	while (true)
	{
		//�մ������̣߳��ȴ���������������񣬷��������ȴ������������������ٻ��ѽ�������
		//����
		pthread_mutex_lock(&(pool->lock));

		/*
		queue_size == 0 ˵��û�����񣬵��� wait ���������������ϣ�
		��������������while
		*/ 
		while ((pool->queue_size == 0) && (!pool->shutdown))
		{
			printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
			//������queue_not_empty������pool->lock����
			pthread_cond_wait(&(pool->queue_empty), &(pool->lock));
			//���ָ����Ŀ�Ŀ����̣߳����Ҫ�������̸߳�������0�������߳�
			if (pool->wait_exit_thr_num > 0)
			{
				pool->wait_exit_thr_num--;
				
				//����̳߳����̸߳���������Сֵʱ���Խ�����ǰ�߳�
				if (pool->live_thr_num > pool->min_thr_num)
				{
					printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
					pool->live_thr_num--;
					pthread_mutex_unlock(&(pool->lock));
					//�߳����н���
					pthread_exit(NULL);	
				}
			}
		}

		//���shutdownΪtrue��Ҫ�ر��̳߳����ÿ���̣߳������˳�����
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&(pool->lock));
			printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
			//�߳����н���
			pthread_exit(NULL);
		}

		//������������ȡ������һ�����Ӳ���
		//�Ӷ���ͷ��ʼ��ȡ����
		task.function = pool->task_queue[pool->queue_front].function;
		task.arg = pool->task_queue[pool->queue_front].arg;

		//���ӣ�ģ�⻷�ζ���,��queue_frontָ�����
		pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
		pool->queue_size--;

		//֪ͨ�������µ�������ӽ���
		pthread_cond_broadcast(&(pool->queue_full));

		//����ȡ�����������̳߳����ͷ�
		pthread_mutex_unlock(&(pool->lock));

		//ִ������
		printf("thread 0x%s start working\n", (unsigned int)pthread_self());
		//Ҫ��æ�߳���������������
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num++;
		pthread_mutex_unlock(&(pool->thread_counter));
		//ִ�лص���������
		(*(task.function))(task.arg);

		//���������
		printf("thread 0x%x end working\n", (unsigned int)pthread_self());
		//�����һ������æ״̬�߳���-1
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num++;
		pthread_mutex_unlock(&(pool->thread_counter));
	}
	pthread_exit(NULL);
}

//�������̵߳Ĺ���
void* adjust_thread(void* arg)
{
	threadpool_t* pool = (threadpool_t*)arg;
	while (!pool->shutdown)
	{
		//ÿʮ����һ��
		sleep(10);
		pthread_mutex_lock(&(pool->lock));
		int queue_size = pool->queue_size;	//���������
		int live_thr_num = pool->live_thr_num;	//�����߳���
		pthread_mutex_unlock(&(pool->lock));

		pthread_mutex_lock(&(pool->thread_counter));
		int busy_thr_num = pool->busy_thr_num;	//æ״̬���߳�����������������Լ�����
		pthread_mutex_unlock(&(pool->thread_counter));

		/*
		����һ�����㷨���жϵ��߳���������ʱ�������µ��߳�
		threadpool_create��ʱ���ǰ���С�߳����������߳�
		�����ǰ���������ڿ��е��߳������Ҵ����߳���С������̸߳���ʱ������
		*/
		if (queue_size > (live_thr_num - busy_thr_num) && live_thr_num < pool->max_thr_num)
		{
			pthread_mutex_lock(&(pool->lock));
			int add = 0;

			//һ���������������߳�
			for (int i = 0; i < pool->max_thr_num && add < queue_size 
							&& pool->live_thr_num < pool->max_thr_num ; i++)
			{
				if (pool->threads[i] == 0 || !is_thread_alive(pool->threads[i]))
				{
					pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool);
					add++;
					pool->live_thr_num++;
				}
			}
			pthread_mutex_unlock(&(pool->lock));
		}
		//���ٶ���Ŀ����̣߳��㷨��æ�߳�*2 С�� �����߳��������Ҵ����߳���������С�߳���ʱ

		if (busy_thr_num * 2 < live_thr_num && live_thr_num > pool->min_thr_num)
		{
			pthread_mutex_lock(&(pool->lock));
			pool->wait_exit_thr_num = live_thr_num - busy_thr_num * 2;
			pthread_mutex_unlock(&(pool->lock));
			//һ�����ٿռ� live_thr_num - busy_thr_num * 2
			for (int i = 0; i < live_thr_num - busy_thr_num * 2; i++)
			{
				//֪ͨ���ڿ���״̬���̣߳����ǻ�������ֹ
				pthread_cond_signal(&(pool->queue_empty));
			}
		}
	}
	return NULL;
}

/*
	�����̳߳�
	min_thr_num����С�߳���
	max_thr_num������߳���
	queue_max_size���������������������߳���Ӧ��һ��
*/
threadpool_t* threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
	if (min_thr_num <= 0)
		min_thr_num = 4;
	if (max_thr_num > 100)
		max_thr_num = 100;
	if (queue_max_size < max_thr_num || queue_max_size > 100)
		queue_max_size = max_thr_num;
	//ʹ�� do{ } while(0) �ķ�ʽ��ִֻ��һ�Σ���������������������ʱ������
	threadpool_t* pool = NULL;

	do 
	{
		//Ϊ�̳߳ط���ռ�
		if ((pool = (threadpool_t*)malloc(sizeof(threadpool_t))) == NULL)
		{
			//����ʧ��
			printf("malloc threadpool fail");
			break;		//����do while
		}
		//��ʼ��
		pool->min_thr_num = min_thr_num;
		pool->max_thr_num = max_thr_num;
		pool->busy_thr_num = 0;
		pool->live_thr_num = min_thr_num;	//���ŵ��߳�������ֵ = ��С�߳���
		pool->queue_size = 0;
		pool->queue_max_size = queue_max_size;
		pool->queue_front = 0;
		pool->queue_rear = 0;
		pool->shutdown = false;				//���ر��̳߳�

		//��������߳����������������߳����鿪�ٿռ䣬������
		pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*max_thr_num);
		if (pool->threads == NULL)
		{
			printf("malloc threads fail");
			break;
		}
		memset(pool->threads, 0, sizeof(pthread_t)*max_thr_num);

		//���п��ٿռ�
		pool->task_queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t)*queue_max_size);
		if (pool->task_queue == NULL)
		{
			printf("malloc task_queue fail");
			break;
		}

		//��ʼ������������������
		if (pthread_mutex_init(&(pool->lock), NULL) != 0
				|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
				|| pthread_cond_init(&(pool->queue_empty), NULL) != 0
				|| pthread_cond_init(&(pool->queue_full), NULL) != 0
			)
		{
			printf("init the lock or cond fail");
			break;
		}

		//���� min_thr_num �������߳�
		for (int i = 0; i < min_thr_num; i++)
		{
			pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool);
			printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
		}
		//�����������߳�
		pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void*)pool);
		
		return pool;
	} while (0);

	//ǰ��������ʧ��ʱ���ͷ�pool�洢�ռ�
	threadpool_free(pool);
}

//���̳߳������һ������
int threadpool_add(threadpool_t* pool, void*(*function)(void*), void* arg)
{
	pthread_mutex_lock(&(pool->lock));

	//==Ϊ�棬�����Ѿ�������wait����
	while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown))
	{
		pthread_cond_wait(&(pool->queue_full), &(pool->lock));
	}
	if (pool->shutdown) {
		pthread_mutex_unlock(&(pool->lock));
	}
	//��չ����̵߳��õĻص������Ĳ��� arg
	if (pool->task_queue[pool->queue_rear].arg != NULL) {
		free(pool->task_queue[pool->queue_rear].arg);
		pool->task_queue[pool->queue_rear].arg = NULL;
	}
	//����������������
	pool->task_queue[pool->queue_rear].function = function;
	pool->task_queue[pool->queue_rear].arg = arg;
	//��βָ���ƶ���ģ�⻷��
	pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
	pool->queue_size++;

	//���������󣬶��в�Ϊ�գ������̳߳��еȴ�����������߳�
	pthread_cond_signal(&(pool->queue_empty));
	pthread_mutex_unlock(&(pool->lock));

	return 0;
}

int threadpool_destroy(threadpool_t* pool)
{
	if (pool == NULL)
		return -1;

	pool->shutdown = true;
	//�����ٹ����߳�
	pthread_join(pool->adjust_tid, NULL);

	for (int i = 0; i < pool.live_thr_num; i++ + )
	{
		//֪ͨ���еĿ����߳�
		pthread_cond_broadcast(&(pool->queue_empty));
	}
	for (int i = 0; i < pool->live_thr_num; i++)
	{
		pthread_join(pool->threads[i], NULL);
	}
	threadpool_free(pool);

	return 0;
}

int threadpool_free(threadpool_t* pool)
{
	if (pool == NULL)
		return -1;
	if (pool->task_queue)
		free(pool->task_queue);
	if (pool->threads)
	{
		free(pool->threads);
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_mutex_lock(&(pool->thread_counter));
		pthread_mutex_destroy(&(pool->thread_counter));
		pthread_cond_destroy(&(pool->queue_empty));
		pthread_cond_destroy(&(pool->queue_full));
	}

	free(pool);
	pool = NULL;
	return 0;
}

int is_thread_alive(pthread_t tid)
{
	//��0���źţ������߳��Ƿ���
	int kill_rc = pthread_kill(tid, 0);	
	if (kill_rc == ESRCH)
		return false;

	return true;
}

int main()
{


	system("pause");
	return 0;
}