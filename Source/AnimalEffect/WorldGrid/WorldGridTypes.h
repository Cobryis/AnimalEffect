// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WorldGridTypes.generated.h"

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FGridPosition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 X;

	UPROPERTY(BlueprintReadWrite)
	int32 Y;

	FGridPosition() : X(-1), Y(-1) {}

	FGridPosition(int32 InX, int32 InY) : X(InX), Y(InY) {}

	FORCEINLINE void Set(int32 InX, int32 InY)
	{
		X = InX; Y = InY;
	}

	FORCEINLINE void Invalidate()
	{
		X = Y = -1;
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("X: %d, Y: %d"), X, Y);
	}

	FORCEINLINE bool IsValid() const
	{
		return (X != -1) && Y != -1;
	}
};

FGridPosition operator+(const FGridPosition& A, const FGridPosition& B);

UENUM()
enum class ETerrainType : uint8
{
	Sand,
	Dirt,
	Rock,
	Water,
	OutOfBounds,
};
