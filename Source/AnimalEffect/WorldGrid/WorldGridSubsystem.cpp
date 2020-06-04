// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridSubsystem.h"

#include "WorldGridInterface.h"
#include "Data/AEDataAsset.h"
#include "Data/DigActualizer.h"

#include "DrawDebugHelpers.h"


DECLARE_LOG_CATEGORY_CLASS(LogWorldGridSubsystem, Log, All);

FGridVector operator+(const FGridVector& A, const FGridVector& B)
{
	return { A.X + B.X, A.Y + B.Y };
}

struct FGridActorAnnotation
{
	FGridVector Position;
	FGridVector Size;

	FGridActorAnnotation() = default;

	// FGridActorAnnotation(const FGridVector& InPosition, const FGridVector& InSize) : Position(InPosition), Size(InSize) {}

	FORCEINLINE bool IsDefault() const { return Position.IsDefault() && Size.IsDefault(); }
	FORCEINLINE bool IsValid() const { return Position.IsValid() && Size.IsValid(); }

};

static FUObjectAnnotationSparse<FGridActorAnnotation, true> GridActorAnnotations;

UWorldGridSubsystem* UWorldGridSubsystem::Get(const UObject* WorldContextObject)
{
	auto WorldGridSubsystem = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)->GetSubsystem<UWorldGridSubsystem>();
	check(WorldGridSubsystem);
	return WorldGridSubsystem;
}

UWorldGridSubsystem::UWorldGridSubsystem()
	: UWorldSubsystem()
{

}

void UWorldGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IWorldGridConfigInterface* Settings = Cast<IWorldGridConfigInterface>(GetWorld()->GetWorldSettings()))
	{
		UE_LOG(LogWorldGridSubsystem, Warning, TEXT("WorldGridSubsystem assumes WorldSettings implements the IWorldGridSettingsInterface."));
		Settings->GetWorldGridConfig(Config);
	}

	int32 GridSize = Config.Width * Config.Width;

	TerrainTypeGrid.SetNum(GridSize);
	ActorGrid.SetNum(GridSize);
	ElevationGrid.SetNum(GridSize);
	DigGrid.SetNum(GridSize);
	DetectionGrid.SetNum(GridSize);

	GridActorAnnotations.Reserve(1000);
}

void UWorldGridSubsystem::Deinitialize()
{
	GridActorAnnotations.RemoveAllAnnotations();
}

bool UWorldGridSubsystem::IsValidPosition(const FGridVector& Position) const
{
	return (Position.X >= 0 && Position.X < Config.Width)
		&& (Position.Y >= 0 && Position.Y < Config.Width);
}

int32 UWorldGridSubsystem::GetElevationAtGridPosition(const FGridVector& Position) const
{
	return IsValidPosition(Position) ? ElevationGrid[GetArrayIndexForGridPosition(Position)] : 0;
}

ETerrainType UWorldGridSubsystem::GetTerrainTypeAtGridPosition(const FGridVector& Position) const
{
	return IsValidPosition(Position) ? TerrainTypeGrid[GetArrayIndexForGridPosition(Position)] : ETerrainType::OutOfBounds;
}

AActor* UWorldGridSubsystem::GetActorAtGridPosition(const FGridVector& Position) const
{
	return IsValidPosition(Position) ? ActorGrid[GetArrayIndexForGridPosition(Position)] : nullptr;
}

TSoftObjectPtr<UDigActualizer> UWorldGridSubsystem::GetDigActualizerAtPosition(const FGridVector& Position) const
{
	return IsValidPosition(Position) ? DigGrid[GetArrayIndexForGridPosition(Position)] : TSoftObjectPtr<UDigActualizer>();
}

TTuple<int32, int32> UWorldGridSubsystem::GetDetectionDataAtPosition(const FGridVector& Position) const
{
	return IsValidPosition(Position) ? DetectionGrid[GetArrayIndexForGridPosition(Position)] : TTuple<int32, int32>();
}

FGridVector UWorldGridSubsystem::GetActorGridSize(AActor* Actor) const
{
	return GridActorAnnotations.GetAnnotation(Actor).Size;
}

bool UWorldGridSubsystem::IsActorOnGrid(AActor* Actor) const
{
	return GridActorAnnotations.GetAnnotation(Actor).IsValid();
}

bool UWorldGridSubsystem::GetActorGridPosition(AActor* Actor, FGridVector& OutPosition) const
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("GetActorGridPosition was provided null"));
		return false;
	}

	return GetGridPositionAtWorldLocation(Actor->GetActorLocation(), OutPosition);
}

// we probably want to allow exceptions in the future for stuff like placing a house on a space where a tree is?
bool UWorldGridSubsystem::IsSpaceUniformAndVacant(const FGridVector& StartPosition, const FGridVector& EndPosition) const
{
	// simple way to make sure all spaces match the elevation and terrain type, just use the start position for reference
	const int32 RequiredElevation = GetElevationAtGridPosition(StartPosition);
	const ETerrainType RequiredTerrainType = GetTerrainTypeAtGridPosition(StartPosition);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridVector CurrentPosition(X, Y);

			// #todo: each of these should generate errors not just return false
			if (!IsValidPosition(CurrentPosition) ||
				(GetElevationAtGridPosition(CurrentPosition) != RequiredElevation) ||
				(GetTerrainTypeAtGridPosition(CurrentPosition) != RequiredTerrainType) ||
				(GetActorAtGridPosition(CurrentPosition) != nullptr))
			{
				return false;
			}
		}
	}

	return true;
}

bool UWorldGridSubsystem::GetVacantPositionAtOrNearPosition(const FGridVector& DesiredPosition, const FGridVector& Size, FGridVector& VacantPosition)
{
	if (IsSpaceUniformAndVacant(DesiredPosition, DesiredPosition + Size))
	{
		VacantPosition = DesiredPosition;
		return true;
	}

	static const TArray<FGridVector> AdjacentOffsets =
	{
		{-1,-1}, {0,-1}, {1,-1},
		{-1, 0},         {1, 0},
		{-1, 1}, {0, 1}, {1, 1}
	};

	for (int32 i = 0; i < AdjacentOffsets.Num(); ++i)
	{
		const FGridVector AdjacentPosition = DesiredPosition + AdjacentOffsets[i];
		if (IsSpaceUniformAndVacant(AdjacentPosition, AdjacentPosition + Size))
		{
			VacantPosition = AdjacentPosition;
			return true;
		}
	}

	return false;
}

bool UWorldGridSubsystem::GetGridPositionAtWorldLocation(const FVector& WorldLocation, FGridVector& OutPosition) const
{
	// first cell is at 0,0. with a scale of 100. cell 1,0 begins at 100,0. cell 0,1 begins at 0,100.

	OutPosition.X = WorldLocation.X / Config.WorldScale;
	OutPosition.Y = WorldLocation.Y / Config.WorldScale;

	return IsValidPosition(OutPosition);
}

FVector UWorldGridSubsystem::GetWorldLocationAtGridPosition(const FGridVector& Position) const
{
	const float CenterOffset = Config.WorldScale / 2.f;

	return GetWorldLocationAtGridPosition_Internal(Position) += FVector(CenterOffset, CenterOffset, 0.f);
}

FVector UWorldGridSubsystem::GetWorldLocationAtGridPosition_Internal(const FGridVector& Position) const
{
	return FVector(Position.X * Config.WorldScale, Position.Y * Config.WorldScale, 0.f);
}

AActor* UWorldGridSubsystem::TrySpawnActorOnGrid(const UAEMetaAsset* ActorAsset, const FWorldGridActorSpawnParameters& GridSpawnParams, TFunction<void(AActor*)> PreFinalizeConstructionCallback)
{
	if (ActorAsset == nullptr)
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't spawn null actor asset on grid"));
		return nullptr;
	}

	if (!ActorAsset->Implements<UWorldGridInterface>())
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't spawn '%s' on grid because it doesn't implement WorldGridInterface"), *ActorAsset->GetName());
		return nullptr;
	}

	if (!IsValidPosition(GridSpawnParams.DesiredPosition))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't spawn '%s' on grid at invalid position: %s"), *ActorAsset->GetName(), *GridSpawnParams.DesiredPosition.ToString());
		return nullptr;
	}

	auto WorldGridInterface = Cast<IWorldGridInterface>(ActorAsset);
	FGridVector ActorSize = WorldGridInterface->GetWorldGridSize();

	if (ActorSize.X < 1 || ActorSize.Y < 1)
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place '%s' on grid because its size is less than 1"), *ActorAsset->GetName());
		return false;
	}

	FGridVector SpawnPosition = GridSpawnParams.DesiredPosition;

	bool bCanPlace;
	if (GridSpawnParams.bCanAdjustPosition)
	{
		bCanPlace = GetVacantPositionAtOrNearPosition(SpawnPosition, ActorSize, SpawnPosition);
	}
	else
	{
		bCanPlace = IsSpaceUniformAndVacant(SpawnPosition, SpawnPosition + ActorSize);
	}

	return bCanPlace
		? SpawnActorOnGrid_Internal(ActorAsset->GetActorClass(), SpawnPosition, ActorSize, GridSpawnParams.Owner, PreFinalizeConstructionCallback)
		: nullptr;
}

AActor* UWorldGridSubsystem::TrySpawnSmallActorOnGrid(TSubclassOf<AActor> ActorClass, const FWorldGridActorSpawnParameters& GridSpawnParams, TFunction<void(AActor*)> PreFinalizeConstructionCallback)
{
	if (ActorClass == nullptr)
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't spawn null actor class on grid"));
		return nullptr;
	}

	if (!IsValidPosition(GridSpawnParams.DesiredPosition))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't spawn '%s' on grid at invalid position: %s"), *ActorClass->GetName(), *GridSpawnParams.DesiredPosition.ToString());
		return nullptr;
	}	
	
	FGridVector SpawnPosition = GridSpawnParams.DesiredPosition;

	// #todo: cleanup FGridVector(1) usage

	bool bCanPlace;
	if (GridSpawnParams.bCanAdjustPosition)
	{
		bCanPlace = GetVacantPositionAtOrNearPosition(SpawnPosition, FGridVector(1), SpawnPosition);
	}
	else
	{
		bCanPlace = IsSpaceUniformAndVacant(SpawnPosition, SpawnPosition + FGridVector(1));
	}
	
	return bCanPlace
		? SpawnActorOnGrid_Internal(ActorClass, SpawnPosition, FGridVector(1), GridSpawnParams.Owner, PreFinalizeConstructionCallback)
		: nullptr;
}

AActor* UWorldGridSubsystem::SpawnActorOnGrid_Internal(TSubclassOf<AActor> ActorClass, const FGridVector& GridPosition, const FGridVector& ActorSize, AActor* Owner, TFunction<void(AActor*)> PreFinalizeConstructionCallback)
{
	check(ActorClass);

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.bDeferConstruction = PreFinalizeConstructionCallback != nullptr;
	ActorSpawnParams.Owner = Owner;

	FTransform SpawnTransform = FTransform(GetWorldLocationAtGridPosition(GridPosition));
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, ActorSpawnParams);

	check(IsValid(SpawnedActor));

	if (ActorSpawnParams.bDeferConstruction)
	{
		PreFinalizeConstructionCallback(SpawnedActor);
		SpawnedActor->FinishSpawning(SpawnTransform);
	}

	GridActorAnnotations.AddAnnotation(SpawnedActor, { GridPosition, ActorSize });
	SetActorAtPositions(SpawnedActor, GridPosition, GridPosition + ActorSize);

	return SpawnedActor;
}

bool UWorldGridSubsystem::RemoveActorFromGrid(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove null actor from grid"));
		return false;
	}

	if (!IsActorOnGrid(Actor))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove '%s' from grid when it wasn't on the grid to begin with"), *Actor->GetName());
		return false;
	}

	FGridActorAnnotation ActorGridAnnotation = GridActorAnnotations.GetAndRemoveAnnotation(Actor);

	check(ActorGridAnnotation.IsValid());
	SetActorAtPositions(nullptr, ActorGridAnnotation.Position, ActorGridAnnotation.Position + ActorGridAnnotation.Size);

	return true;
}

bool UWorldGridSubsystem::TryPlaceDigActualizerOnGrid(TSoftObjectPtr<UDigActualizer> Actualizer, const FGridVector& DesiredPosition)
{
	// we need to load this to know how to place it. #fixme
	Actualizer.LoadSynchronous();

	if (!Actualizer.IsValid())
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place invalid DigActualizer on grid"));
		return false;
	}

	if (!IsValidPosition(DesiredPosition))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place '%s' on grid at invalid position '%s'"), *Actualizer->GetName(), *DesiredPosition.ToString());
		return false;
	}

	SetDigActualizerAtPosition(Actualizer, DesiredPosition);

	// #hack: set detection data. this will mess up detection for overlapping detection data
	for (int32 Y = -Actualizer->DetectionRadius; Y <= Actualizer->DetectionRadius; ++Y)
	{
		for (int32 X = -Actualizer->DetectionRadius; X <= Actualizer->DetectionRadius; ++X)
		{
			const FGridVector CurrentPosition = DesiredPosition + FGridVector(X, Y);
			if (IsValidPosition(CurrentPosition))
			{
				// maybe come up with a smarter way to calculate distance
				const int32 Distance = FMath::Max(FMath::Abs(X), FMath::Abs(Y));
				const TTuple<int32, int32> DetectionData(Actualizer->DetectionRarity, Distance);
				SetDetectionDataAtPosition(DetectionData, CurrentPosition);
			}
		}
	}

	return true;
}

TSoftObjectPtr<UDigActualizer> UWorldGridSubsystem::TryRemoveDigActualizerFromGrid(const FGridVector& Position)
{
	if (!IsValidPosition(Position))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove from grid at invalid position '%s'"), *Position.ToString());
		return false;
	}

	TSoftObjectPtr<UDigActualizer> Actualizer = GetDigActualizerAtPosition(Position);

	// #hack: nuke the detection grid. this will mess up detection for overlapping detection data
	if (Actualizer.IsValid())
	{
		const TTuple<int32, int32> EmptyData(0, 0);
		for (int32 Y = -Actualizer->DetectionRadius; Y <= Actualizer->DetectionRadius; ++Y)
		{
			for (int32 X = -Actualizer->DetectionRadius; X <= Actualizer->DetectionRadius; ++X)
			{
				const FGridVector CurrentPosition = Position + FGridVector(X, Y);
				if (IsValidPosition(CurrentPosition))
				{
					SetDetectionDataAtPosition(EmptyData, CurrentPosition);
				}
			}
		}
	}

	return Actualizer;
}

void UWorldGridSubsystem::DebugDrawPosition(const FGridVector& Position, float DisplayTime, const FColor& Color)
{
	FVector DrawLocation = GetWorldLocationAtGridPosition(Position);
	DrawDebugSphere(GetWorld(), DrawLocation, 50.f, 8, Color, false, DisplayTime);
}

void UWorldGridSubsystem::SetActorAtPositions(AActor* Actor, const FGridVector& StartPosition, const FGridVector& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridVector CurrentPosition(X, Y);
			ActorGrid[GetArrayIndexForGridPosition(CurrentPosition)] = Actor;
		}
	}
}

void UWorldGridSubsystem::SetTerrainTypeAtPositions(ETerrainType TerrainType, const FGridVector& StartPosition, const FGridVector& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridVector CurrentPosition(X, Y);
			TerrainTypeGrid[GetArrayIndexForGridPosition(CurrentPosition)] = TerrainType;
		}
	}
}

void UWorldGridSubsystem::SetElevationAtPositions(int32 Elevation, const FGridVector& StartPosition, const FGridVector& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridVector CurrentPosition(X, Y);
			ElevationGrid[GetArrayIndexForGridPosition(CurrentPosition)] = Elevation;
		}
	}
}

void UWorldGridSubsystem::SetDigActualizerAtPosition(TSoftObjectPtr<UDigActualizer> DigActualizer, const FGridVector& Position)
{
	check(IsValidPosition(Position));

	DigGrid[GetArrayIndexForGridPosition(Position)] = DigActualizer;
}

void UWorldGridSubsystem::SetDetectionDataAtPosition(const TTuple<int32, int32>& DetectionData, const FGridVector& Position)
{
	check(IsValidPosition(Position));

	DetectionGrid[GetArrayIndexForGridPosition(Position)] = DetectionData;
}

int32 UWorldGridSubsystem::GetArrayIndexForGridPosition(const FGridVector& Position) const
{
	if (IsValidPosition(Position))
	{
		return (Position.Y * Config.Width) + Position.X;
	}
	else
	{
		return INDEX_NONE;
	}
}

