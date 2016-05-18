/**	
 *	Creates a pool of threads, which will service a queue of funtors.
 * 
 *	The pool of threads is determined by the MAX_THREADS define, or passed in through the constructor.   
 *  To have a thread  execute a request, the user submits a ThreadRequest derived command 
 *  object (functor) to the thread queue.  (I was going to call it threadJob but that's rude).
 *  The request handler will pull this out of the queue, and run the functors operator() method.
 *  If the queue is full, determined by the MAX_QUEUE_SIZE define, or the constructor parameter, 
 *  then the submitRequest method will block preventing new items being added to the queue.  
 *  These are implemented via a ManualEvent object.
 *
 *	@warning Exceptions WILL NOT propogate out of the accept handler.  You MUST catch 
 *           your exceptions in the ThreadRequest derived functor.
 *
 *	@author Peter Hancock
 */
#pragma once

//线程任务
class CThreadRequest
{
public:
	//任务执行函数，必须自己处理异常
	virtual void ThreadProc(unsigned int nThreadId) = 0;	
};

class CThreadPool  
{
public:
	//线程开始参数传递结构
	struct ThreadData
	{
		CThreadPool *pThreadPool;	// this
		unsigned int nThreadId;		//线程id
	};
	class SuicideException
	{
	};
	class SuicidePill : public CThreadRequest
	{
	public:
		virtual void ThreadProc(unsigned int)
		{
			throw SuicideException();
		}
	};

public:
	enum { THREAD_COUNT = 5, MAX_QUEUE_SIZE = 100 };

	//线程池构造函数
	// threadSize - 线程数
	// queueSize  - 任务队列大小
	CThreadPool(int threadSize=THREAD_COUNT, int queueSize=MAX_QUEUE_SIZE);
	virtual ~CThreadPool();

	//线程池开始工作
	int						Begin(void);
	//加入工作要求，不用考虑CThreadRequest的内存释放
	//返回false表示对列满或者已经关闭
	BOOL					PushRequest(CThreadRequest* pRequest);
	//关闭
	void					ShutDown(void);
	//
	int						GetThreadCount(void) const { return (int)m_vThreadPool.size(); }
	//杀死僵死的线程，并补充一个新的线程进入池中
	bool					tryKillThread(unsigned int threadID);

private:
	//线程池是否处于激活中，只有在激活状态才能接受任务
	BOOL							m_bAlive;
	//线程句柄池
	std::vector< HANDLE >			m_vThreadPool;
	//线程ID池
	std::vector< unsigned int >		m_vThreadIDPool;
	//任务队列
	std::queue< CThreadRequest* >	m_dJobQueue;
	//任务队列最大尺寸
	int								m_nQueueSize;
	//任务队列限制信号量，用来确保每一个任务只有一个线程接收到
	HANDLE							m_hQueueAccessSemaphone;
	//用来确定队列是线程安全的
	CRITICAL_SECTION				m_csQueue;
	//用来确定队列有空余的位置
	HANDLE							m_hQueueNotFullEvent;

	static unsigned int __stdcall _ThreadProc(void* lpParam);
	void ThreadProc(unsigned int nThreadId) throw();
};

