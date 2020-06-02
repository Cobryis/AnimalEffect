// Copyright Epic Games, Inc. All Rights Reserved.

#include "AEWorldSettings.h"

AAEWorldSettings::AAEWorldSettings()
{

}

void AAEWorldSettings::GetWorldGridConfig(FWorldGridConfig& OutConfig)
{
	OutConfig = WorldGrid;
}

