// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemAsset.h"

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
