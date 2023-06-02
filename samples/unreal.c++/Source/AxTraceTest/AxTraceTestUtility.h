// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AxTraceTestUtility.generated.h"

/**
 * 
 */
UCLASS()
class AXTRACETEST_API UAxTraceTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static int64 GetObjectUniqueID(const UObject* Object);

	UFUNCTION(BlueprintCallable)
	static void TestMultithreadAxLog();

};
