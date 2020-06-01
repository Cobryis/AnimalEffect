// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Inventory.generated.h"

class UAEMetaAsset;

UENUM(BlueprintType)
enum class EInventoryType : uint8
{
	Item,
	Tool
};

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FInventorySlotData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	const UAEMetaAsset* AssetType = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 StackSize = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 Quality = 0;

	FORCEINLINE bool IsValid() const { return AssetType && (StackSize > 0) && (Quality > 0); }
};


USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FInventory
{
	GENERATED_BODY()

	// all these properties should probably be private

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FInventorySlotData> Slots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RowCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Column Count"))
	int32 ColCount = 10;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY()
	bool bOneSlotPerAssetType = false;

	UPROPERTY(BlueprintReadOnly)
	int32 Size;

	void InitializeInventory(AActor* Owner, EInventoryType InventoryType, bool bInOneSlotPerClass = false);

	// returns a remainder of items not added
	int32 TryAdd(const UAEMetaAsset* AssetType, int32 Count = 1, uint8 Quality = 1);

	// returns true if found and removed a valid item
	bool TryRemoveAtIndex(int32 Index, FInventorySlotData& RemovedItem);

	bool TryRemoveSingleAtIndex(int32 Index, FInventorySlotData& RemovedItem);

	// returns true if found a valid item
	bool GetAtIndex(int32 Index, FInventorySlotData& Item) const;

	int32 GetAssetCount(const UAEMetaAsset* AssetType) const;
};
