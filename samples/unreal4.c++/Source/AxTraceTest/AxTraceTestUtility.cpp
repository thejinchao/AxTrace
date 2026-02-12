// Fill out your copyright notice in the Description page of Project Settings.
#include "AxTraceTestUtility.h"
#include "AxTraceFunctionLibrary.h"

int64 UAxTraceTestUtility::GetObjectUniqueID(const UObject* Object)
{
	if (Object == nullptr || !IsValid(Object))
	{
		return INDEX_NONE;
	}

	return (int64)(Object->GetUniqueID());
}

class FAxLogWorker : public IQueuedWork
{
public:
	FAxLogWorker(const FString& message) : Message(message)
	{
	}

	virtual ~FAxLogWorker() override
	{
	}

	virtual void DoThreadedWork() override
	{
		FPlatformProcess::Sleep(0.01f);
		UAxTrace::Log(Message);
		// Finish the task, delete the worker
		delete this;
	}

	virtual void Abandon() override
	{
		delete this;
	}
public:
	FString Message;
};


void UAxTraceTestUtility::TestMultithreadAxLog()
{
	int blank_Count = 0;
	int step = 1;
	int MAX_BLANK_COUNT = 50;

	// Create A Thread Pool
	FQueuedThreadPool* Pool = FQueuedThreadPool::Allocate();
	Pool->Create(8, 0, TPri_Normal, TEXT("AxLogThreadPool"));

	for (int i = 0; i < 500; i++)
	{
		char szTemp[1024] = { 0 };

		int j = 0;
		for (j = 0; j < blank_Count; j++) szTemp[j] = L' ';

		blank_Count += step;
		if (blank_Count >= MAX_BLANK_COUNT) step = -1;
		if (blank_Count <= 0)step = 1;

		szTemp[j++] = L'*';
		szTemp[j++] = 0;

		// Worker will be deleted when the job is done
		Pool->AddQueuedWork(new FAxLogWorker(FString(szTemp)));
	}
}