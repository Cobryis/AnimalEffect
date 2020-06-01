// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tool.h"

UToolAsset::UToolAsset()
	: UItemAsset()
{
	bCanStack = false;

	InventoryType = EInventoryType::Tool;
	InventoryActions = { EInventoryItemActions::Hold, EInventoryItemActions::Drop };
}


ATool::ATool()
	: AItem()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
}

void ATool::OnEquipped()
{
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
}

void ATool::OnUnequipped()
{
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void ATool::OnActivate()
{

}
