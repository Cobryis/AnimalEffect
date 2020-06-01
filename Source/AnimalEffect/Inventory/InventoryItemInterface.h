// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory.h"

#include "CoreMinimal.h"

#include "InventoryItemInterface.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemActions : uint8
{
	Hold,
	Place,
	Drop,
	Split,
	Stack
};

UINTERFACE()
class ANIMALEFFECT_API UInventoryItemInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IInventoryItemInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual uint8 GetInventoryStackMax() const = 0;

	virtual uint8 GetItemQuality() const { return 1; }

	// this returns a reference so we know this array exists on the class
	// #todo: this should end up context aware
	virtual bool GetAvailableItemActions(TArray<EInventoryItemActions>& AvailableActions) const { return false; }

	virtual EInventoryType GetInventoryType() const = 0;

};
