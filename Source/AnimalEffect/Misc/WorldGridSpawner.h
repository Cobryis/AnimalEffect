// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Data/AEDataAsset.h"

#include "GameFramework/Actor.h"

#include "WorldGridSpawner.generated.h"

class UWorldGridSubsystem;
struct FGridVector;

UCLASS(NotBlueprintable, Abstract)
class AWorldGridSpawner : public AActor
{
	GENERATED_BODY()

public:

	AWorldGridSpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void TriggerSpawner();

protected:

	void BeginPlay() override;

private:

	void TrySpawn();

protected:

	virtual bool TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition) PURE_VIRTUAL(AWorldGridSpawner::TrySpawn_Internal, return false;);

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bCanAdjust = false;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bTriggerOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnTimer = 0;

	UPROPERTY(EditAnywhere, Category = "Lifespan")
	bool bDestroyOnSpawn = true;

};

UCLASS()
class AWorldGridAssetSpawner : public AWorldGridSpawner
{
	GENERATED_BODY()

protected:

	bool TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition) override;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	UAEMetaAsset* AssetToSpawn = nullptr;

};

UCLASS()
class AWorldGridSmallActorSpawner : public AWorldGridSpawner
{
	GENERATED_BODY()

protected:

	bool TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition) override;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AActor> ActorClassToSpawn = nullptr;

};