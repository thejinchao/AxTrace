// Copyright Epic Games, Inc. All Rights Reserved.

#include "AxTraceTestGameMode.h"
#include "AxTraceTestHUD.h"
#include "AxTraceTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAxTraceTestGameMode::AAxTraceTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AAxTraceTestHUD::StaticClass();
}
