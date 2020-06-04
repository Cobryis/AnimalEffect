// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridTypes.h"

#include "Subsystems/WorldSubsystem.h"

#include "WorldGridSubsystem.generated.h"

class UAEMetaAsset;

USTRUCT()
struct FWorldGridConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = 2, ClampMax = 1024))
	int32 Width = 100;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 10, ClampMax = 500))
	float WorldScale = 100.f;

};

USTRUCT(BlueprintType)
struct FWorldGridActorSpawnParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGridVector DesiredPosition;

	UPROPERTY(BlueprintReadWrite)
	AActor* Owner = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bCanAdjustPosition = false;

};

class UDigActualizer;

/**
 * Breaks the world up into a grid.
 * - At a given grid position, there can only ever be one elevation, one terrain type, and at most one actor.
 * - A single actor can take up multiple grid positions.
 * - An actor can have on itself a smaller grid that can have other actors on it, thus
 *   bending the rule that there can only be one actor at a given position.
 * The first cell of the grid starts at 0,0 in world space. Every subsequent cell is in the positive x or y direction.
 * We may want to turn this into an interface and pImpl so as to reduce tight coupling like between DigActualizer and WorldGrid.
 */
UCLASS()
class ANIMALEFFECT_API UWorldGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UWorldGridSubsystem* Get(const UObject* WorldContextObject);

	UWorldGridSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	bool IsValidPosition(const FGridVector& Position) const;

	// 0 is sea level. 1 would be 1 cliff height above level, not the difference in height between sand and dirt.
	int32 GetElevationAtGridPosition(const FGridVector& Position) const;
	ETerrainType GetTerrainTypeAtGridPosition(const FGridVector& Position) const;
	AActor* GetActorAtGridPosition(const FGridVector& Position) const;
	TSoftObjectPtr<UDigActualizer> GetDigActualizerAtPosition(const FGridVector& Position) const;
	TTuple<int32, int32> GetDetectionDataAtPosition(const FGridVector& Position) const;

	// returns an invalid GridVector if actor is not on grid
	FGridVector GetActorGridSize(AActor* Actor) const;

	// whether this actor is stored on the grid
	bool IsActorOnGrid(AActor* Actor) const;

	// returns true if the position is a valid position on the grid. to see if the actor is actually stored on the grid, use IsActorOnGrid
	bool GetActorGridPosition(AActor* Actor, FGridVector& OutPosition) const;

	bool IsSpaceUniformAndVacant(const FGridVector& StartPosition, const FGridVector& EndPosition) const;
	bool GetVacantPositionAtOrNearPosition(const FGridVector& CurrentPosition, const FGridVector& Size, FGridVector& VacantPosition);

	// returns true if this position is on the grid
	bool GetGridPositionAtWorldLocation(const FVector& WorldLocation, FGridVector& OutPosition) const;

	// returns a world location that is at the center of the grid position
	FVector GetWorldLocationAtGridPosition(const FGridVector& Position) const;

	AActor* TrySpawnActorOnGrid(const UAEMetaAsset* ActorAsset, const FWorldGridActorSpawnParameters& GridSpawnParams, TFunction<void(AActor*)> PreFinalizeConstructionCallback = nullptr);
	AActor* TrySpawnSmallActorOnGrid(TSubclassOf<AActor> ActorClass, const FWorldGridActorSpawnParameters& GridSpawnParams, TFunction<void(AActor*)> PreFinalizeConstructionCallback = nullptr);

	bool RemoveActorFromGrid(AActor* Actor);

	bool TryPlaceDigActualizerOnGrid(TSoftObjectPtr<UDigActualizer> Actualizer, const FGridVector& DesiredPosition);
	TSoftObjectPtr<UDigActualizer> TryRemoveDigActualizerFromGrid(const FGridVector& Position);

	void DebugDrawPosition(const FGridVector& Position, float DisplayTime, const FColor& Color);

private:

	FVector GetWorldLocationAtGridPosition_Internal(const FGridVector& Position) const;

	AActor* SpawnActorOnGrid_Internal(TSubclassOf<AActor> ActorClass, const FGridVector& GridPosition, const FGridVector& ActorSize, AActor* Owner, TFunction<void(AActor*)> PreFinalizeConstructionCallback);

	void SetActorAtPositions(AActor* Actor, const FGridVector& StartPosition, const FGridVector& EndPosition);
	void SetTerrainTypeAtPositions(ETerrainType TerrainType, const FGridVector& StartPosition, const FGridVector& EndPosition);
	void SetElevationAtPositions(int32 Elevation, const FGridVector& StartPosition, const FGridVector& EndPosition);
	void SetDigActualizerAtPosition(TSoftObjectPtr<UDigActualizer> DigActualizer, const FGridVector& Position);
	void SetDetectionDataAtPosition(const TTuple<int32, int32>& DetectionData, const FGridVector& Position);

	int32 GetArrayIndexForGridPosition(const FGridVector& Position) const;

	FWorldGridConfig Config;

	TArray<int32> ElevationGrid;
	TArray<ETerrainType> TerrainTypeGrid;
	TArray<AActor*> ActorGrid;
	TArray<TSoftObjectPtr<UDigActualizer>> DigGrid;
	TArray<TTuple<int32,int32>> DetectionGrid;
};

UINTERFACE()
class ANIMALEFFECT_API UWorldGridConfigInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IWorldGridConfigInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual void GetWorldGridConfig(FWorldGridConfig& OutConfig) = 0;

};
