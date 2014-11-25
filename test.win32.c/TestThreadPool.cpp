#include "StdAfx.h"
#include "TestThreadPool.h"


//--------------------------------------------------------------------------------------------
CThreadPool::CThreadPool(int threadSize, int queueSize)
{
	m_bAlive = FALSE;
	m_vThreadPool.resize(threadSize);
	m_vThreadIDPool.resize(threadSize);

	m_nQueueSize = queueSize;

	m_hQueueAccessSemaphone = ::CreateSemaphore(NULL, 0, queueSize, NULL);

	if(!m_hQueueAccessSemaphone)
		throw std::exception("Can't Create QueueAccessSemaphone");
	::InitializeCriticalSection(&m_csQueue);

	m_hQueueNotFullEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_hQueueNotFullEvent == NULL)
		throw std::exception("Can't Create QueueNotFullEvent");
	//表示队列不满
	::SetEvent(m_hQueueNotFullEvent);
}

//--------------------------------------------------------------------------------------------
CThreadPool::~CThreadPool()
{
	if(m_hQueueNotFullEvent)
		::CloseHandle(m_hQueueNotFullEvent);
	::DeleteCriticalSection(&m_csQueue);
	if(m_hQueueAccessSemaphone)
		::CloseHandle(m_hQueueAccessSemaphone);
}

//--------------------------------------------------------------------------------------------
int	CThreadPool::Begin(void)
{
	if(!m_bAlive)
	{
		for(int i=0; i<(int)m_vThreadPool.size(); i++)
		{
			try
			{
				ThreadData *pData = new ThreadData;
				assert(pData);

				//创建线程
				pData->pThreadPool = this;
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProc, pData, CREATE_SUSPENDED, &(pData->nThreadId));
				if(hThread)
					m_vThreadPool[i] = hThread;
				m_vThreadIDPool[i] = pData->nThreadId;
			}
			catch(...)
			{
				assert(false);
			}
		}
		//开始线程
		std::for_each(m_vThreadPool.begin(), m_vThreadPool.end(), ::ResumeThread);
		m_bAlive = TRUE;
	}

	return (int)m_vThreadPool.size();
}

//--------------------------------------------------------------------------------------------
bool CThreadPool::tryKillThread(unsigned int threadID)
{
	//暂停线程
	std::for_each(m_vThreadPool.begin(), m_vThreadPool.end(), ::SuspendThread);

	bool replace = false;
	//查找线程
	for(int i=0; i<(int)m_vThreadPool.size(); i++)
	{
		if(threadID == m_vThreadIDPool[i])
		{
			//终止该线程
			::TerminateThread(m_vThreadPool[i], 0);

			ThreadData *pData = new ThreadData;
			assert(pData);

			//创建线程
			pData->pThreadPool = this;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProc, pData, CREATE_SUSPENDED, &(pData->nThreadId));
			if(hThread)
				m_vThreadPool[i] = hThread;
			m_vThreadIDPool[i] = pData->nThreadId;

			replace = true;
			break;
		}
	}

	//开始线程
	std::for_each(m_vThreadPool.begin(), m_vThreadPool.end(), ::ResumeThread);

	return replace;
}

//--------------------------------------------------------------------------------------------
unsigned int __stdcall CThreadPool::_ThreadProc(void* lpParam)
{
	ThreadData *pData = static_cast<ThreadData*>(lpParam);
	try
	{
		pData->pThreadPool->ThreadProc(pData->nThreadId);
	}
	catch(...)
	{
	}
	delete pData;				
	_endthreadex(0);
	return 0; 
}

//--------------------------------------------------------------------------------------------
void CThreadPool::ThreadProc(unsigned int nThreadId) throw()
{
	//开始线程
	while(TRUE)
	{
		//确保只有一个线程接收到该任务
		::WaitForSingleObject(m_hQueueAccessSemaphone, INFINITE);
		try
		{
			//进入关键段
			::EnterCriticalSection(&m_csQueue);
			//有任务
			if(!m_dJobQueue.empty())
			{
				std::auto_ptr< CThreadRequest > pJob(m_dJobQueue.front());
				m_dJobQueue.pop();
				//确认队列不满
				::SetEvent(m_hQueueNotFullEvent);
				//离开关键段
				::LeaveCriticalSection(&m_csQueue);
				//执行任务
				try
				{
					pJob->ThreadProc(nThreadId);
				}
				catch(CThreadPool::SuicideException&) //使用异常让线程退出
				{
					break;
				}
			}
			else
			{
				::LeaveCriticalSection(&m_csQueue);
			}
		}
		catch(...)
		{
			//MessageBox(NULL, "意外的异常发生，请重新尝试!", NULL, MB_OK|MB_ICONSTOP);
			::LeaveCriticalSection(&m_csQueue);
		}

		//任务结束，察看是否有下一个任务
	}
}

//--------------------------------------------------------------------------------------------
BOOL CThreadPool::PushRequest(CThreadRequest* pRequest)
{
	assert(pRequest);
	if(!pRequest) return FALSE;
	//已经关闭
	if(!m_bAlive) return FALSE;

	//队列满
	if(WAIT_OBJECT_0 != ::WaitForSingleObject(m_hQueueNotFullEvent, 0))
	{
		return FALSE;
	}

	try
	{
		//进入关键段
		::EnterCriticalSection(&m_csQueue);
		m_dJobQueue.push(pRequest);
		if((int)m_dJobQueue.size() >= m_nQueueSize)
			::ResetEvent(m_hQueueNotFullEvent);

		//信号量减一
		::ReleaseSemaphore(m_hQueueAccessSemaphone, 1, NULL);
		//离开关键段
		::LeaveCriticalSection(&m_csQueue);
		return TRUE;
	}
	catch(...)
	{
		::LeaveCriticalSection(&m_csQueue);
		//接着抛出
		throw;
	}
}

//--------------------------------------------------------------------------------------------
void CThreadPool::ShutDown(void)
{
	//加入自杀任务
	std::vector<HANDLE>::const_iterator it;
	for(it = m_vThreadPool.begin(); it != m_vThreadPool.end(); ++it)
	{
		if(!PushRequest(new CThreadPool::SuicidePill()))
		{
			//队列已满，退出
			break;
		}
	}
	m_bAlive = FALSE;

	//强制关闭线程
	for(it = m_vThreadPool.begin() ; it != m_vThreadPool.end() ; ++it)
	{
		switch(::WaitForSingleObject(*it, 10))
		{
		case WAIT_OBJECT_0:
			// This is what we WANT to happen.
			break;

		case WAIT_TIMEOUT:			// Something bad happened - force the termination
			::TerminateThread(*it, 1);
			break;

		case WAIT_ABANDONED:	    // The thread has been terminated already!?  maybe...
			break;
		}
		::CloseHandle(*it);
	}

	//清空线程
	m_vThreadPool.clear();
	m_vThreadIDPool.clear();
}
