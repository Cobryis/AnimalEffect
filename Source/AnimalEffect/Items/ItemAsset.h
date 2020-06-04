// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Data/AEDataAsset.h"
#include "Interfaces/DropInterface.h"
#include "Inventory/InventoryItemInterface.h"

#include "ItemAsset.generated.h"

class UStaticMesh;

UCLASS(Abstract)
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
