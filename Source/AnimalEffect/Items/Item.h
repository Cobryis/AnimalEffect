// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Data/AEDataAsset.h"
#include "Interfaces/DropInterface.h"
#include "Interfaces/PickupActorInterface.h"
#include "Interfaces/InteractableActorInterface.h"
#include "Inventory/InventoryItemInterface.h"
#include "WorldGrid/WorldGridInterface.h"

#include "Gameframework/Actor.h"

#include "Item.generated.h"

class UStaticMesh;

UCLASS()
class ANIMALEFFECT_API UItemAsset : public UAEMetaAsset
	, public IInventoryItemInterface
	, public IDropInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UStaticMesh* PickupMesh;

	UPROPERTY()
	EInventoryType InventoryType;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	bool bCanStack = true;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = 1, ClampMax = 255, EditCondition = bCanStack))
	int32 InventoryStackMax = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TArray<EInventoryItemActions> InventoryActions;

public:

	UItemAsset();

	// BEGIN IDropInterface
	FORCEINLINE UStaticMesh* GetDropMesh() const override { return PickupMesh; }
	FORCEINLINE FText GetDropDescription() const override { return GetTitle(); }
	// END IDropInterface

	// BEGIN IInventoryItemInterface
	FORCEINLINE uint8 GetInventoryStackMax() const override { return bCanStack ? InventoryStackMax : 1; }
	bool GetAvailableItemActions(TArray<EInventoryItemActions>& AvailableActions) const override;
	EInventoryType GetInventoryType() const { return InventoryType; }
	// END IInventoryItemInterface
};


UCLASS(Abstract)
class ANIMALEFFECT_API AItem : public AActor						   
	, public IInteractableActorInterface		   
	, public IPickupActorInterface			   
	, public IWorldGridActorInterface	// #todo: potentially move this to the ItemAsset class when we refactor WorldGridInterface	  
{
	GENERATED_BODY()

public:

	AItem();

	// BEGIN IInteractableInterface
	void Interact(APawn* InteractInstigator) const override {}
	// END InteractableInterface

	// BEGIN IPickupInterface
	void GetPickupData(FPickupData& OutPickupData) const;
	bool CanPickup(APawn* InteractInstigator) const override { return true; }
	// END IPickupInterface

	// BEGIN IWorldGridInterface
	void GetWorldGridSize(int32& X, int32& Y) const override;
	void SetWorldGridPosition(const FGridPosition& InGridPosition) override;
	void GetWorldGridPosition(FGridPosition& OutGridPosition) const override;
	// END IWorldGridInterface

protected:

	UPROPERTY(EditDefaultsOnly, Category = "World Grid", meta = (ClampMin = 1, ClampMax = 20))
	int32 PlacedSizeX = 1;

	UPROPERTY(EditDefaultsOnly, Category = "World Grid", meta = (ClampMin = 1, ClampMax = 20))
	int32 PlacedSizeY = 1;

	UPROPERTY(BlueprintReadOnly, Category = "World Grid")
	FGridPosition GridPosition;

};
