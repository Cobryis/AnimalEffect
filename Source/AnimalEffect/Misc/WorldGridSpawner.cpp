// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridSpawner.h"

#include "WorldGrid/WorldGridSubsystem.h"

DECLARE_LOG_CATEGORY_CLASS(LogWorldGridSpawner, Log, All);

AWorldGridSpawner::AWorldGridSpawner()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	// UBillboardComponent* IconComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
}

void AWorldGridSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bTriggerOnBeginPlay)
	{
		TriggerSpawner();
	}
}

void AWorldGridSpawner::TriggerSpawner()
{
	if (SpawnTimer > 0.f)
	{
		FTimerHandle SpawnTimerHandle;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle,this, &AWorldGridSpawner::TrySpawn, SpawnTimer);
	}
	else
	{
		TrySpawn();
	}
}

void AWorldGridSpawner::TrySpawn()
{
	UWorldGridSubsystem* WorldGrid = UWorldGridSubsystem::Get(this);

	FGridVector GridPosition;
	if (WorldGrid->GetGridPositionAtWorldLocation(GetActorLocation(), GridPosition))
	{
		if (TrySpawn_Internal(WorldGrid, GridPosition))
		{
			if (bDestroyOnSpawn)
			{
				Destroy();
			}
		}
		else
		{
			UE_LOG(LogWorldGridSpawner, Warning, TEXT("'%s' failed to spawn, check log messages above this one."), *GetName());
		}
	}
	else
	{
		UE_LOG(LogWorldGridSpawner, Warning, TEXT("'%s' is not on WorldGrid!"), *GetName());
	}
}

bool AWorldGridAssetSpawner::TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition)
{
	if (AssetToSpawn)
	{
		FWorldGridActorSpawnParameters SpawnParams;
		SpawnParams.bCanAdjustPosition = bCanAdjust;
		SpawnParams.DesiredPosition = DesiredPosition;
		return IsValid(WorldGrid->TrySpawnActorOnGrid(AssetToSpawn, SpawnParams));
	}
	else
	{
		UE_LOG(LogWorldGridSpawner, Warning, TEXT("'%s' has null AssetToSpawn!"), *GetName());
		return false;
	}
}

bool AWorldGridSmallActorSpawner::TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition)
{
	if (ActorClassToSpawn)
	{
		FWorldGridActorSpawnParameters SpawnParams;
		SpawnParams.bCanAdjustPosition = bCanAdjust;
		SpawnParams.DesiredPosition = DesiredPosition;
		return IsValid(WorldGrid->TrySpawnSmallActorOnGrid(ActorClassToSpawn, SpawnParams));
	}
	else
	{
		UE_LOG(LogWorldGridSpawner, Warning, TEXT("'%s' has null ActorClassToSpawn!"), *GetName());
		return false;
	}
}
