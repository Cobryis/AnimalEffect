// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/PickupActorInterface.h"
#include "WorldGrid/WorldGridInterface.h"

#include "GameFramework/Actor.h"

#include "DropActor.generated.h"

UCLASS()
class ANIMALEFFECT_API ADropActor : public AActor, public IWorldGridActorInterface, public IPickupActorInterface
{
	GENERATED_BODY()

public:

	ADropActor();

	static ADropActor* NewDrop(const UObject* WorldContextObject, const FGridPosition& SpawnPosition, const FPickupData& PickupData, APawn* Instigator);

	// BEGIN IWorldGridInterface
	void GetWorldGridSize(int32& X, int32& Y) const override { X = Y = 1; }
	void SetWorldGridPosition(const FGridPosition& InGridPosition) override;
	void GetWorldGridPosition(FGridPosition& OutGridPosition) const override;
	// END IWorldGridInterface

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

	UPROPERTY(BlueprintReadOnly, Category = "World Grid")
	FGridPosition GridPosition;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* MeshComponent;

};

// utility class for placing a drop in the world in the editor
UCLASS()
class ANIMALEFFECT_API ADropSpawner : public AActor
{
	GENERATED_BODY()

public:

	ADropSpawner();

	void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Drop")
	FPickupData PickupData;

};
