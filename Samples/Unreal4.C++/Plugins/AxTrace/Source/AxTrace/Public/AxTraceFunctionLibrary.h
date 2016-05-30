/*
axia|trace3
Copyright(C) thecodeway.com
*/
#pragma once

#include "Engine.h"
#include "Core.h"

#include "AxTraceFunctionLibrary.generated.h"

UENUM(BlueprintType) 
enum class AxTraceStyleEnum : uint8
{
	AXT_TRACE=0	UMETA(DisplayName="Trace"),
	AXT_DEBUG=1	UMETA(DisplayName = "Debug"),
	AXT_INFO=2	UMETA(DisplayName = "Info"),
	AXT_WARN=3	UMETA(DisplayName = "Warn"),
	AXT_ERROR=4	UMETA(DisplayName = "Error"),
	AXT_FATAL=5	UMETA(DisplayName = "Fatal"),

	AXT_USERDEFINE=10	UMETA(DisplayName = "UserDefine"),
};

/**
 * AxTrace
 */
UCLASS()
class AXTRACE_API UAxTraceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "1", DisplayName = "AxLog"), Category = "AxTrace")
	static void AxLog(const FString& String= FString(TEXT("Hello")), AxTraceStyleEnum Style= AxTraceStyleEnum::AXT_TRACE, int UserDefineStyle = 0);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "2", DisplayName = "AxValue"), Category = "AxTrace")
	static void AxValue(const FString& Name = FString(TEXT("Name")), const FString& Value = FString(TEXT("Value")), AxTraceStyleEnum Style = AxTraceStyleEnum::AXT_TRACE, int UserDefineStyle = 0);

private:
	struct AxTraceContext;

	static AxTraceContext* _get_thread_context(void);
	static AxTraceContext* _try_init(void);
};

