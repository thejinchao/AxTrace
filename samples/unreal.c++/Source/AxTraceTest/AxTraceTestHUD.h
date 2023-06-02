// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AxTraceTestHUD.generated.h"

UCLASS()
class AAxTraceTestHUD : public AHUD
{
	GENERATED_BODY()

public:
	AAxTraceTestHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

