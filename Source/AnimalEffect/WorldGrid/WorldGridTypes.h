// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WorldGridTypes.generated.h"

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FGridVector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y;

	constexpr FGridVector() : X(-1), Y(-1) {}

	constexpr FGridVector(int32 InX, int32 InY) : X(InX), Y(InY) {}

	constexpr FGridVector(int32 InDimension) : X(InDimension), Y(InDimension) {}

	FORCEINLINE void Set(int32 InX, int32 InY)
	{
		X = InX; Y = InY;
	}

	FORCEINLINE void Invalidate()
	{
		*this = FGridVector();
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("X: %d, Y: %d"), X, Y);
	}

	FORCEINLINE bool IsDefault() const { return (X == -1) && (Y == -1); }

	FORCEINLINE bool IsValid() const
	{
		return !IsDefault();
	}
};

FGridVector operator+(const FGridVector& A, const FGridVector& B);

UENUM()
enum class ETerrainType : uint8
{
	Sand,
	Dirt,
	Rock,
	Water,
	OutOfBounds,
};
