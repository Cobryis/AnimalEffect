// Copyright Epic Games, Inc. All Rights Reserved.

#include "DigActualizer.h"

#include "WorldGrid/WorldGridSubsystem.h"

DECLARE_LOG_CATEGORY_CLASS(LogDigActualizer, Log, All)

FPickupData UDigActualizer::Actualize() const
{
	if (DigPickups.Num() > 0)
	{
		return DigPickups[FMath::RandRange(0, DigPickups.Num() - 1)];
	}

	return FPickupData();
}

bool ADigActualizerSpawner::TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition)
{
	return WorldGrid->TryPlaceDigActualizerOnGrid(Actualizer, DesiredPosition);
}
