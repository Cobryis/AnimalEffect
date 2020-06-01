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

ADigActualizerSpawner::ADigActualizerSpawner()
	: AActor()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ADigActualizerSpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorldGridSubsystem* UGS = GetWorld()->GetSubsystem<UWorldGridSubsystem>();

	FGridPosition GridPosition;
	if (UGS->GetGridPositionAtWorldLocation(GetActorLocation(), GridPosition))
	{
		UGS->TryPlaceDigActualizerOnGrid(Actualizer, GridPosition);
	}
	else
	{
		UE_LOG(LogDigActualizer, Warning, TEXT("DigActualizer spawner '%s' cannot find position on grid to spawn drop"), *GetName());
	}
}
