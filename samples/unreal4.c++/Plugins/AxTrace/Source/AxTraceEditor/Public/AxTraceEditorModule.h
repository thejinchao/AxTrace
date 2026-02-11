/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAxTraceEditorModule : public IModuleInterface
{
public:
	// IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
