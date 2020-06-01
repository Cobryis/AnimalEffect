// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item.h"

#include "Data/AEDataAsset.h"

UItemAsset::UItemAsset()
	: UAEMetaAsset()
{
	InventoryType = EInventoryType::Item;
	InventoryActions = { EInventoryItemActions::Place, EInventoryItemActions::Drop };
}

bool UItemAsset::GetAvailableItemActions(TArray<EInventoryItemActions>& AvailableActions) const
{
	AvailableActions = InventoryActions;
	return true;
}


AItem::AItem()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

// for picking up a single instance of this item
void AItem::GetPickupData(FPickupData& OutPickupData) const
{
	OutPickupData.AssetType = UAEMetaAsset::GetMetaAssetForClass(GetClass());
	OutPickupData.Quality = 1; // #todo
	OutPickupData.StackSize = 1;
}

void AItem::GetWorldGridSize(int32& X, int32& Y) const
{
	X = PlacedSizeX;
	Y = PlacedSizeY;
}

void AItem::SetWorldGridPosition(const FGridPosition& InGridPosition)
{
	GridPosition = InGridPosition;
}

void AItem::GetWorldGridPosition(FGridPosition& OutGridPosition) const
{
	OutGridPosition = GridPosition;
}
