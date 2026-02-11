/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/
#pragma once

#include "CoreMinimal.h"
#include "AxTraceFunctionLibrary.h"
#include "AxTraceSettings.generated.h"


UCLASS(config = Game, DefaultConfig, DisplayName="AxTrace")
class UAxTraceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Whether the AxTrace functions is enabled */
	UPROPERTY(config, EditAnywhere, Category = Setting, DisplayName = "Enable AxTrace")
	bool EnableAxTrace = true;

	/** The AxTrace Receiver Address */
	UPROPERTY(config, EditAnywhere, Category = Receiver, meta = (EditCondition = "EnableAxTrace", EditConditionHides))
	FString ReceiverAddress = DEFAULT_AXTRACE_RECEIVER_IP;

	/** The AxTrace Receiver Port */
	UPROPERTY(config, EditAnywhere, Category = Receiver, meta = (EditCondition = "EnableAxTrace", EditConditionHides))
	int ReceiverPort = DEFAULT_AXTRACE_RECEIVER_PORT;
};
