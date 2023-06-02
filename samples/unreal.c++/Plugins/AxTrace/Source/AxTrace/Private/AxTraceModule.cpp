/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/

#include "AxTraceModule.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AxTraceSettings.h"
#include "AxTraceFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FAxTraceModule"

void FAxTraceModule::StartupModule()
{
	// load dependencies
	if (FModuleManager::Get().LoadModule(TEXT("Networking")) == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("The required module 'Networking' failed to load. Plug-in 'AxTrace' cannot be used."));

		return;
	}

	//Access config via 'class default object'
	GeAxTraceConfig();
}

void FAxTraceModule::ShutdownModule()
{

}

const UAxTraceSettings* FAxTraceModule::GeAxTraceConfig() const
{
	if (!CachedSettings)
	{
		CachedSettings = GetDefault<UAxTraceSettings>();
	}
	return CachedSettings;
}

void UAxTraceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FAxTraceModule::Get()->IncresePIEID();
}

void UAxTraceSubsystem::Deinitialize()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAxTraceModule, AxTrace)