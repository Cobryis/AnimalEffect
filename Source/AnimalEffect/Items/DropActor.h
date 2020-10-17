// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/PickupActorInterface.h"
#include "Misc/WorldGridSpawner.h"

#include "GameFramework/Actor.h"

#include "DropActor.generated.h"

UCLASS(NotBlueprintable)
class ANIMALEFFECT_API ADropActor : public AActor
	, public IPickupActorInterface
{
	GENERATED_BODY()

public:

	ADropActor();

	static ADropActor* NewDrop(const UObject* WorldContextObject, const FGridVector& SpawnPosition, const FPickupData& PickupData, APawn* Instigator);

	// BEGIN IPickupInterface
	bool CanPickup(APawn* PawnInstigator) const override { return true; }
	void GetPickupText(FText& OutPickupText) const override { OutPickupText = DropText; }
	void GetPickupData(FPickupData& OutPickupData) const override { OutPickupData = PickupData; }
	void UpdatePickupStackSize(uint8 NewStackSize) override { PickupData.StackSize = NewStackSize; }
	// END IPickupInterface

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Drop")
	FPickupData PickupData;

	UPROPERTY(BlueprintReadOnly, Category = "Drop")
	FText DropText;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* MeshComponent;

};

// utility class for placing a drop in the world in the editor
UCLASS()
class ANIMALEFFECT_API ADropSpawner : public AWorldGridSpawner
{
	GENERATED_BODY()

protected:

	bool TrySpawn_Internal(UWorldGridSubsystem* WorldGrid, const FGridVector& DesiredPosition) override;

private:

	UPROPERTY(EditAnywhere, Category = "Drop")
	FPickupData PickupData;

};
