// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlaceableActor.h"

APlaceableActor::APlaceableActor()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

// for picking up a single instance of this item
void APlaceableActor::GetPickupData(FPickupData& OutPickupData) const
{
	OutPickupData.AssetType = UAEMetaAsset::GetMetaAssetForClass(GetClass());
	OutPickupData.Quality = 1; // #todo
	OutPickupData.StackSize = 1;
}
