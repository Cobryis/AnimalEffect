// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ItemAsset.h"
#include "Interfaces/PickupActorInterface.h"
#include "Interfaces/InteractableActorInterface.h"
#include "WorldGrid/WorldGridInterface.h"

#include "Gameframework/Actor.h"

#include "PlaceableActor.generated.h"

UCLASS()
class ANIMALEFFECT_API UPlaceableAsset : public UItemAsset
	, public IWorldGridInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "World Grid", meta = (ClampMin = 1, ClampMax = 10))
	int32 PlacedSizeX = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "World Grid", meta = (ClampMin=1, ClampMax=10))
	int32 PlacedSizeY = 1;

public:

	// BEGIN IWorldGridInterface
	FORCEINLINE FGridVector GetWorldGridSize() const override { return { PlacedSizeX, PlacedSizeY }; }
	// END IWorldGridInterface
};


UCLASS(Abstract)
class ANIMALEFFECT_API APlaceableActor : public AActor						   
	, public IInteractableActorInterface		   
	, public IPickupActorInterface			    
{
	GENERATED_BODY()

public:

	APlaceableActor();

	// BEGIN IInteractableInterface
	void Interact(APawn* InteractInstigator) const override {}
	// END InteractableInterface

	// BEGIN IPickupInterface
	void GetPickupData(FPickupData& OutPickupData) const;
	bool CanPickup(APawn* InteractInstigator) const override { return true; }
	// END IPickupInterface

};
