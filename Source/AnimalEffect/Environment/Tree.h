// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Data/AEDataAsset.h"
#include "WorldGrid/WorldGridInterface.h"

#include "GameFramework/Actor.h"

#include "Tree.generated.h"

UCLASS()
class ANIMALEFFECT_API UTreeAsset : public UAEMetaAsset
	, public IWorldGridInterface
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "World Grid", meta = (ClampMin = 1, ClampMax = 2))
	int32 Size = 1;

public:

	// BEGIN IWorldGridInterface
	FGridVector GetWorldGridSize() const { return FGridVector(Size); }
	// END IWorldGridInterface

};

UCLASS()
class ANIMALEFFECT_API ATree : public AActor
{
	GENERATED_BODY()

public:

	ATree();

	void OnAxeHit(APawn* HitInstigator);

private:

	UPROPERTY(BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	int32 Health;

};
