// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory.h"

#include "InventoryItemInterface.h"
#include "Data/AEDataAsset.h"

DECLARE_LOG_CATEGORY_CLASS(LogInventory, Log, All);

void FInventory::InitializeInventory(AActor* Owner, EInventoryType InventoryType, bool bInOneSlotPerAssetType)
{
	check(Owner);

	Name = UEnum::GetValueAsString(InventoryType);
	bOneSlotPerAssetType = bInOneSlotPerAssetType;
	Size = RowCount * ColCount;

	Slots.SetNum(Size);

	UE_LOG(LogInventory, Log, TEXT("Initialized '%s' Inventory with size %d for '%s'."), *Name, Slots.Num(), *Owner->GetName());
}

namespace
{
	int32 TryAddToEmptySlot(const UAEMetaAsset* AssetType, int32 Count, uint8 Quality, uint8 StackMax, FInventory& Inventory)
	{
		uint8 CountRemaining = Count;
		for (FInventorySlotData& CurrentSlot : Inventory.Slots)
		{
			if (CurrentSlot.AssetType == nullptr)
			{
				check(CurrentSlot.StackSize == 0); // we should never have a null item with a value

				CurrentSlot.AssetType = AssetType;
				CurrentSlot.Quality = Quality;

				const uint8 AmountAdded = FMath::Min(CountRemaining, StackMax);
				CurrentSlot.StackSize = AmountAdded;
				CountRemaining -= AmountAdded;

				check(CountRemaining >= 0);

				if (CountRemaining == 0)
				{
					break;
				}
			}
		}

		return CountRemaining;
	}
}

int32 FInventory::TryAdd(const UAEMetaAsset* AssetType, int32 Count, uint8 Quality)
{
	if (AssetType == nullptr)
	{
		UE_LOG(LogInventory, Error, TEXT("Can't add null asset to inventory '%s'"), *Name);
		return Count;
	}

	if (!AssetType->Implements<UInventoryItemInterface>())
	{
		UE_LOG(LogInventory, Error, TEXT("Can't add '%s' to inventory '%s'"), *AssetType->GetName(), *Name);
		return Count;
	}

	if (Quality <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("Can't add '%s' with quality '%d' to inventory '%s'"), *AssetType->GetName(), Quality, *Name);
		return Count;
	}

	if (Count <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("Can't add '%s' of count '%d' to inventory '%s'"), *AssetType->GetName(), Count, *Name);
		return Count;
	}

	check(Slots.Num() == (RowCount * ColCount));

	const int32 StackMax = Cast<IInventoryItemInterface>(AssetType)->GetInventoryStackMax();

	bool bAttemptedAddToExistingStack = false;

	int32 CountRemaining = Count;
	// add to existing
	for (FInventorySlotData& CurrentSlot : Slots)
	{
		// must be an exact match
		if ((CurrentSlot.AssetType == AssetType) && (CurrentSlot.Quality == Quality))
		{
			bAttemptedAddToExistingStack = true;

			const int32 SpaceAvailable = StackMax - CurrentSlot.StackSize;
			if (SpaceAvailable > 0)
			{
				const uint8 AmountAdded = static_cast<uint8>(FMath::Min(CountRemaining, SpaceAvailable));
				CurrentSlot.StackSize += AmountAdded;
				CountRemaining -= AmountAdded;

				check(CountRemaining >= 0);

				if ((CountRemaining == 0) || bOneSlotPerAssetType)
				{
					break;
				}
			}
		}
	}

	if (bOneSlotPerAssetType && bAttemptedAddToExistingStack)
	{
		return CountRemaining;
	}

	const bool bNoneAdded = CountRemaining == Count;
	// if we're either not limited to one slot per asset type or none have been added yet, then add to an empty slot
	if ((CountRemaining > 0) && (!bOneSlotPerAssetType || bNoneAdded))
	{
		return TryAddToEmptySlot(AssetType, CountRemaining, Quality, StackMax, *this);
	}
	else
	{
		return CountRemaining;
	}
}

bool FInventory::TryRemoveAtIndex(int32 Index, FInventorySlotData& RemovedItem)
{
	if (GetAtIndex(Index, RemovedItem))
	{
		Slots[Index] = FInventorySlotData();
		return true;
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("Attempted to remove invalid item at index %d in inventory '%s'"), Index, *Name);
		return false;
	}
}

bool FInventory::TryRemoveSingleAtIndex(int32 Index, FInventorySlotData& RemovedItem)
{
	if (GetAtIndex(Index, RemovedItem))
	{
		RemovedItem.StackSize = 1;
		Slots[Index].StackSize -= 1;
		if (Slots[Index].StackSize == 0)
		{
			Slots[Index] = FInventorySlotData();
		}
		return true;
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("Attempted to remove invalid item at index %d in inventory '%s'"), Index, *Name);
		return false;
	}
}

bool FInventory::GetAtIndex(int32 Index, FInventorySlotData& Item) const
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogInventory, Error, TEXT("Can't get item from invalid index %d in inventory '%s'"), Index, *Name);
		return false;
	}

	Item = Slots[Index];
	return Item.IsValid();
}


int32 FInventory::GetAssetCount(const UAEMetaAsset* Asset) const
{
	int32 Count = 0;
	for (const FInventorySlotData& CurrentSlot : Slots)
	{
		if (CurrentSlot.AssetType == Asset)
		{
			Count += CurrentSlot.StackSize;
		}
	}

	return Count;
}
