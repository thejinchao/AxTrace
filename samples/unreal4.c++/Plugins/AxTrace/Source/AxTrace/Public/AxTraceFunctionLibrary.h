/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/
#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"

#include "AxTraceFunctionLibrary.generated.h"

#define DEFAULT_AXTRACE_RECEIVER_IP		"127.0.0.1"
#define DEFAULT_AXTRACE_RECEIVER_PORT	(1978)

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
class AXTRACE_API UAxTrace : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "1", DisplayName = "AxLog"), Category = "AxTrace")
	static void Log(const FString& String = FString(TEXT("Hello")), AxTraceStyleEnum Style = AxTraceStyleEnum::AXT_INFO, int UserDefineStyle = 0);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_Boolean(FString Name, bool Value);

	static void Value_Int8(FString Name, int8 Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_UInt8(FString Name, uint8 Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_Int32(FString Name, int32 Value);

	static void Value_UInt32(FString Name, uint32 Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_Int64(FString Name, int64 Value);

	static void Value_UInt64(FString Name, uint64 Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_Float(FString Name, float Value);

	static void Value_Double(FString Name, double Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_Name(FString Name, FName Value);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static void Value_String(FString Name, FString Value);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "1", DisplayName = "AxScene2D Begin"), Category = "AxTrace")
	static void Scene2DBegin(const FString& SceneName = FString(TEXT("SceneName")), float xMin=0, float yMin=0, float xMax=256.f, float yMax=256.f, const FString& SceneDefine= FString(TEXT("")));

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "AxScene2D Actor"), Category = "AxTrace")
	static void Scene2DActor(const FString& SceneName = FString(TEXT("SceneName")), int64 ActorID=0, const FVector2D& Position= FVector2D(0,0), float Direction=0, int32 ActorStyle=0, const FString& ActorInfo = FString(TEXT("")));

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "AxScene2D End"), Category = "AxTrace")
	static void Scene2DEnd(const FString& SceneName = FString(TEXT("SceneName")));

private:
	struct AxTraceContext;
	static AxTraceContext* GetContext();

	static AxTraceContext* CreateContext();
	static void ResetContext(AxTraceContext* ctx);
	static void SendShakehandMessage(AxTraceContext* ctx);

	static void _Value(const FString& Name, const void* Value, unsigned int ValueType, int ValueSize);
};

