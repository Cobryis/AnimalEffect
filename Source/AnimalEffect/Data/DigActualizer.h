// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Items/Interfaces/PickupActorInterface.h" // #note: the fact that we're not using the interface here suggests we shouldn't include this

#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"

#include "DigActualizer.generated.h"

// this class actualizes something that was buried
// #todo: figure out if WorldGrid actually needs to know about this asset
// or if we can abstract it in some way.. DetectionRadius/Rarity is the problem
UCLASS()
class ANIMALEFFECT_API UDigActualizer : public UDataAsset
{
	GENERATED_BODY()

public:

	// radius in grid cells that the buried item can be detected
	UPROPERTY(EditAnywhere, Category = "Detection")
	int32 DetectionRadius;

	// the detected rarity of the buried item
	UPROPERTY(EditAnywhere, Category = "Rarity")
	int32 DetectionRarity;

	// this is probably temporary
	// any of the items in this class have an equal chance of spawning
	// might want to replace this with FDigData
	UPROPERTY(EditAnywhere, Category = "Items")
	TArray<FPickupData> DigPickups;

	FPickupData Actualize() const;

};

UCLASS()
class ANIMALEFFECT_API ADigActualizerSpawner : public AActor
{
	GENERATED_BODY()

public:

	ADigActualizerSpawner();

	void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Items")
	TSoftObjectPtr<UDigActualizer> Actualizer;

};