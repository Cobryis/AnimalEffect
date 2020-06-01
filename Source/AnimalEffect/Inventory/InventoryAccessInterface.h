// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory.h"

#include "InventoryItemInterface.h" // #hack: this is only included for EInventoryItemActions

#include "InventoryAccessInterface.generated.h"

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FInventorySlotHandle
{
	GENERATED_BODY()

	UPROPERTY()
	EInventoryType InventoryType;

	UPROPERTY()
	int32 SlotIndex;

};

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FInventoryLayoutData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 NumColumns;

	UPROPERTY(BlueprintReadOnly)
	int32 NumRows;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInventorySlotHandle> SlotHandles;
};

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class ANIMALEFFECT_API UInventoryAccessInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IInventoryAccessInterface : public IInterface
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool GetInventoryLayoutData(EInventoryType InventoryType, FInventoryLayoutData& LayoutData) const PURE_VIRTUAL(IInventoryAccessInterface::GetInventoryLayoutData, return false;);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool GetInventorySlotFromHandle(const FInventorySlotHandle& Handle, FInventorySlotData& OutData) const PURE_VIRTUAL(IInventoryAccessInterface::GetInventorySlotFromHandle, return false;);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool GetInventorySlotActionsFromHandle(const FInventorySlotHandle& Handle, TArray<EInventoryItemActions>& OutMenuOptions) const PURE_VIRTUAL(IInventoryAccessInterface::GetInventorySlotMenuOptionsFromHandle, return false;);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void OnInventorySlotActionSelectedForHandle(const FInventorySlotHandle& Handle, EInventoryItemActions Action) PURE_VIRTUAL(IInventoryAccessInterface::OnInventorySlotMenuOptionSelectedForHandle, );

};