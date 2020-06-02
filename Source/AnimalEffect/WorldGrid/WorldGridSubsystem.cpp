// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridSubsystem.h"

#include "WorldGridInterface.h"
#include "Data/DigActualizer.h"

#include "DrawDebugHelpers.h"


DECLARE_LOG_CATEGORY_CLASS(LogWorldGridSubsystem, Log, All);

FGridPosition operator+(const FGridPosition& A, const FGridPosition& B)
{
	return { A.X + B.X, A.Y + B.Y };
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
}

bool UWorldGridSubsystem::IsValidPosition(const FGridPosition& Position) const
{
	return (Position.X >= 0 && Position.X < Config.Width)
		&& (Position.Y >= 0 && Position.X < Config.Width);
}

int32 UWorldGridSubsystem::GetElevationAtGridPosition(const FGridPosition& Position) const
{
	return IsValidPosition(Position) ? ElevationGrid[GetArrayIndexForGridPosition(Position)] : 0;
}

ETerrainType UWorldGridSubsystem::GetTerrainTypeAtGridPosition(const FGridPosition& Position) const
{
	return IsValidPosition(Position) ? TerrainTypeGrid[GetArrayIndexForGridPosition(Position)] : ETerrainType::OutOfBounds;
}

AActor* UWorldGridSubsystem::GetActorAtGridPosition(const FGridPosition& Position) const
{
	return IsValidPosition(Position) ? ActorGrid[GetArrayIndexForGridPosition(Position)] : nullptr;
}

TSoftObjectPtr<UDigActualizer> UWorldGridSubsystem::GetDigActualizerAtPosition(const FGridPosition& Position) const
{
	return IsValidPosition(Position) ? DigGrid[GetArrayIndexForGridPosition(Position)] : TSoftObjectPtr<UDigActualizer>();
}

TTuple<int32, int32> UWorldGridSubsystem::GetDetectionDataAtPosition(const FGridPosition& Position) const
{
	return IsValidPosition(Position) ? DetectionGrid[GetArrayIndexForGridPosition(Position)] : TTuple<int32, int32>();
}

// we probably want to allow exceptions in the future for stuff like placing a house on a space where a tree is?
bool UWorldGridSubsystem::IsSpaceUniformAndVacant(const FGridPosition& StartPosition, const FGridPosition& EndPosition) const
{
	// simple way to make sure all spaces match the elevation and terrain type, just use the start position for reference
	const int32 RequiredElevation = GetElevationAtGridPosition(StartPosition);
	const ETerrainType RequiredTerrainType = GetTerrainTypeAtGridPosition(StartPosition);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridPosition CurrentPosition(X, Y);

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

bool UWorldGridSubsystem::GetVacantPositionAtOrNearPosition(const FGridPosition& DesiredPosition, const FGridPosition& Size, FGridPosition& VacantPosition)
{
	if (IsSpaceUniformAndVacant(DesiredPosition, DesiredPosition + Size))
	{
		VacantPosition = DesiredPosition;
		return true;
	}

	static const TArray<FGridPosition> AdjacentOffsets =
	{
		{-1,-1}, {0,-1}, {1,-1},
		{-1, 0},         {1, 0},
		{-1, 1}, {0, 1}, {1, 1}
	};

	for (int32 i = 0; i < AdjacentOffsets.Num(); ++i)
	{
		const FGridPosition AdjacentPosition = DesiredPosition + AdjacentOffsets[i];
		if (IsSpaceUniformAndVacant(AdjacentPosition, AdjacentPosition + Size))
		{
			VacantPosition = AdjacentPosition;
			return true;
		}
	}

	return false;
}

bool UWorldGridSubsystem::GetGridPositionAtWorldLocation(const FVector& WorldLocation, FGridPosition& OutPosition) const
{
	// first cell is at 0,0. with a scale of 100. cell 1,0 begins at 100,0. cell 0,1 begins at 0,100.

	OutPosition.X = WorldLocation.X / Config.WorldScale;
	OutPosition.Y = WorldLocation.Y / Config.WorldScale;

	return IsValidPosition(OutPosition);
}

void UWorldGridSubsystem::GetWorldLocationAtGridPosition(const FGridPosition& Position, FVector& OutWorldLocation) const
{
	OutWorldLocation.X = Position.X * Config.WorldScale;
	OutWorldLocation.Y = Position.Y * Config.WorldScale;
	OutWorldLocation.Z = 0.f;
}

void UWorldGridSubsystem::GetWorldLocationCenteredAtGridPosition(const FGridPosition& Position, FVector& OutWorldLocation) const
{
	GetWorldLocationAtGridPosition(Position, OutWorldLocation);
	
	const float CenterOffset = Config.WorldScale / 2.f;

	OutWorldLocation.X += CenterOffset;
	OutWorldLocation.Y += CenterOffset;
}

bool UWorldGridSubsystem::TryPlaceActorOnGrid(AActor* Actor, const FGridPosition& DesiredPosition, bool bCanAdjust, FGridPosition& FinalPosition)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place null actor on grid"));
		return false;
	}

	if (!Actor->GetClass()->ImplementsInterface(UWorldGridActorInterface::StaticClass()))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place '%s' on grid because it doesn't implement the WorldGridInterface"), *Actor->GetName());
		return false;
	}

	if (!IsValidPosition(DesiredPosition))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place '%s' on grid because provided grid position '%s' was invalid"), *Actor->GetName(), *DesiredPosition.ToString());
		return false;
	}

	checkSlow(!ActorGrid.Contains(Actor));

	FGridPosition ActorSize;
	IWorldGridActorInterface* WorldGridActor = Cast<IWorldGridActorInterface>(Actor);
	WorldGridActor->GetWorldGridSize(ActorSize.X, ActorSize.Y);

	if (ActorSize.X < 1 || ActorSize.Y < 1)
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't place '%s' on grid because its size is less than 1"), *Actor->GetName());
		return false;
	}

	bool bCanPlace;
	if (bCanAdjust)
	{
		bCanPlace = GetVacantPositionAtOrNearPosition(DesiredPosition, ActorSize, FinalPosition);
	}
	else
	{
		bCanPlace = IsSpaceUniformAndVacant(DesiredPosition, DesiredPosition + ActorSize);
	}

	if (bCanPlace)
	{
		SetActorAtPositions(Actor, FinalPosition, FinalPosition + ActorSize);
		WorldGridActor->SetWorldGridPosition(FinalPosition);
	}

	return bCanPlace;
}

bool UWorldGridSubsystem::RemoveActorFromGrid(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove null actor from grid"));
		return false;
	}

	if (!Actor->GetClass()->ImplementsInterface(UWorldGridActorInterface::StaticClass()))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove '%s' from grid because it doesn't implement the WorldGridInterface"), *Actor->GetName());
		return false;
	}

	IWorldGridActorInterface* WorldGridActor = Cast<IWorldGridActorInterface>(Actor);
	FGridPosition ActorPosition;
	WorldGridActor->GetWorldGridPosition(ActorPosition);
	if (!ActorPosition.IsValid() || !IsValidPosition(ActorPosition))
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove '%s' from grid because it doesn't have a Valid position"), *Actor->GetName());
		return false;
	}

	FGridPosition ActorSize;
	WorldGridActor->GetWorldGridSize(ActorSize.X, ActorSize.Y);

	if (ActorSize.X < 1 || ActorSize.Y < 1)
	{
		UE_LOG(LogWorldGridSubsystem, Error, TEXT("Can't remove '%s' from grid because its size is less than 1"), *Actor->GetName());
		return false;
	}

	SetActorAtPositions(nullptr, ActorPosition, ActorPosition + ActorSize);
	WorldGridActor->SetWorldGridPosition(FGridPosition());

	return true;
}

bool UWorldGridSubsystem::TryPlaceDigActualizerOnGrid(TSoftObjectPtr<UDigActualizer> Actualizer, const FGridPosition& DesiredPosition)
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
			const FGridPosition CurrentPosition = DesiredPosition + FGridPosition(X, Y);
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

TSoftObjectPtr<UDigActualizer> UWorldGridSubsystem::TryRemoveDigActualizerFromGrid(const FGridPosition& Position)
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
				const FGridPosition CurrentPosition = Position + FGridPosition(X, Y);
				if (IsValidPosition(CurrentPosition))
				{
					SetDetectionDataAtPosition(EmptyData, CurrentPosition);
				}
			}
		}
	}

	return Actualizer;
}

void UWorldGridSubsystem::DebugDrawPosition(const FGridPosition& Position, float DisplayTime, const FColor& Color)
{
	FVector DrawLocation;
	GetWorldLocationCenteredAtGridPosition(Position, DrawLocation);
	DrawDebugSphere(GetWorld(), DrawLocation, 50.f, 8, Color, false, DisplayTime);
}

void UWorldGridSubsystem::SetActorAtPositions(AActor* Actor, const FGridPosition& StartPosition, const FGridPosition& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridPosition CurrentPosition(X, Y);
			ActorGrid[GetArrayIndexForGridPosition(CurrentPosition)] = Actor;
		}
	}
}

void UWorldGridSubsystem::SetTerrainTypeAtPositions(ETerrainType TerrainType, const FGridPosition& StartPosition, const FGridPosition& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridPosition CurrentPosition(X, Y);
			TerrainTypeGrid[GetArrayIndexForGridPosition(CurrentPosition)] = TerrainType;
		}
	}
}

void UWorldGridSubsystem::SetElevationAtPositions(int32 Elevation, const FGridPosition& StartPosition, const FGridPosition& EndPosition)
{
	check(IsValidPosition(StartPosition));
	check(IsValidPosition(EndPosition));
	check(StartPosition.X < EndPosition.X);
	check(StartPosition.Y < EndPosition.Y);

	for (int32 Y = StartPosition.Y; Y < EndPosition.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < EndPosition.X; ++X)
		{
			const FGridPosition CurrentPosition(X, Y);
			ElevationGrid[GetArrayIndexForGridPosition(CurrentPosition)] = Elevation;
		}
	}
}

void UWorldGridSubsystem::SetDigActualizerAtPosition(TSoftObjectPtr<UDigActualizer> DigActualizer, const FGridPosition& Position)
{
	check(IsValidPosition(Position));

	DigGrid[GetArrayIndexForGridPosition(Position)] = DigActualizer;
}

void UWorldGridSubsystem::SetDetectionDataAtPosition(const TTuple<int32, int32>& DetectionData, const FGridPosition& Position)
{
	check(IsValidPosition(Position));

	DetectionGrid[GetArrayIndexForGridPosition(Position)] = DetectionData;
}

int32 UWorldGridSubsystem::GetArrayIndexForGridPosition(const FGridPosition& Position) const
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

