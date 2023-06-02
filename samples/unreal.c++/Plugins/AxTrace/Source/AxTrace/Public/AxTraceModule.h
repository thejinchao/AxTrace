/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/

#pragma once

#include "Modules/ModuleManager.h"
#include "AxTraceModule.generated.h"

class UAxTraceSettings;

class FAxTraceModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FAxTraceModule* Get()
	{
		FAxTraceModule* axtraceModule = FModuleManager::GetModulePtr<FAxTraceModule>("AxTrace");
		if (axtraceModule == nullptr)
		{
			axtraceModule = &(FModuleManager::LoadModuleChecked<FAxTraceModule>("AxTrace"));
		}
		return axtraceModule;
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	const UAxTraceSettings* GeAxTraceConfig() const;

	int GetPIEID() const { return PIE_ID; }
	void IncresePIEID() { PIE_ID++; }
private:
	mutable const class UAxTraceSettings* CachedSettings;
	int PIE_ID = 0;
};

UCLASS()
class UAxTraceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//override function from UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Deinitialize();
};