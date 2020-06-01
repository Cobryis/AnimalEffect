// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridTypes.h"

#include "Subsystems/WorldSubsystem.h"

#include "WorldGridSubsystem.generated.h"

USTRUCT()
struct FWorldGridConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = 2, ClampMax = 1024))
	int32 Width = 100;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 10, ClampMax = 500))
	float WorldScale = 100.f;

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

	UWorldGridSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsValidPosition(const FGridPosition& Position) const;

	// 0 is sea level. 1 would be 1 cliff height above level, not the difference in height between sand and dirt.
	int32 GetElevationAtGridPosition(const FGridPosition& Position) const;
	ETerrainType GetTerrainTypeAtGridPosition(const FGridPosition& Position) const;
	AActor* GetActorAtGridPosition(const FGridPosition& Position) const;
	TSoftObjectPtr<UDigActualizer> GetDigActualizerAtPosition(const FGridPosition& Position) const;
	TTuple<int32, int32> GetDetectionDataAtPosition(const FGridPosition& Position) const;

	bool IsSpaceUniformAndVacant(const FGridPosition& StartPosition, const FGridPosition& EndPosition) const;
	bool GetVacantPositionAtOrNearPosition(const FGridPosition& CurrentPosition, const FGridPosition& Size, FGridPosition& VacantPosition);

	bool GetGridPositionAtWorldLocation(const FVector& WorldLocation, FGridPosition& OutPosition) const;
	void GetWorldLocationAtGridPosition(const FGridPosition& Position, FVector& OuWorldLocation) const;
	void GetWorldLocationCenteredAtGridPosition(const FGridPosition& Position, FVector& OutWorldLocation) const;

	bool TryPlaceActorOnGrid(AActor* Actor, const FGridPosition& DesiredPosition, bool bCanAdjust, FGridPosition& FinalPosition);
	bool RemoveActorFromGrid(AActor* Actor);

	bool TryPlaceDigActualizerOnGrid(TSoftObjectPtr<UDigActualizer> Actualizer, const FGridPosition& DesiredPosition);
	TSoftObjectPtr<UDigActualizer> TryRemoveDigActualizerFromGrid(const FGridPosition& Position);

	void DebugDrawPosition(const FGridPosition& Position, float DisplayTime, const FColor& Color);

private:

	void SetActorAtPositions(AActor* Actor, const FGridPosition& StartPosition, const FGridPosition& EndPosition);
	void SetTerrainTypeAtPositions(ETerrainType TerrainType, const FGridPosition& StartPosition, const FGridPosition& EndPosition);
	void SetElevationAtPositions(int32 Elevation, const FGridPosition& StartPosition, const FGridPosition& EndPosition);
	void SetDigActualizerAtPosition(TSoftObjectPtr<UDigActualizer> DigActualizer, const FGridPosition& Position);
	void SetDetectionDataAtPosition(const TTuple<int32, int32>& DetectionData, const FGridPosition& Position);

	int32 GetArrayIndexForGridPosition(const FGridPosition& Position) const;

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
