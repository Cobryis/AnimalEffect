// Copyright Epic Games, Inc. All Rights Reserved.

#include "AEGameMode.h"

#include "AEHUD.h"
#include "AEPlayerController.h"

AAEGameMode::AAEGameMode()
	: AGameModeBase()
{
	HUDClass = AAEHUD::StaticClass();
	PlayerControllerClass = AAEPlayerController::StaticClass();
}

