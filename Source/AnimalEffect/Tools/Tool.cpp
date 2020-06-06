// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tool.h"

#include "WorldGrid/WorldGridSubsystem.h"

UToolAsset::UToolAsset()
	: UItemAsset()
{
	bCanStack = false;

	InventoryType = EInventoryType::Tool;
	InventoryActions = { EInventoryItemActions::Hold, EInventoryItemActions::Drop };
}


ATool::ATool()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false; // tick will be called by AECharacter

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
}

void ATool::OnEquipped()
{
	WorldGrid = UWorldGridSubsystem::Get(this);
	SetActorHiddenInGame(false);
}

void ATool::OnUnequipped()
{
	SetActorHiddenInGame(true);
}

void ATool::StartAction(const FOnToolFinishAction& OnFinishActionCallback)
{
	OnFinishAction = OnFinishActionCallback;

	// #todo: maybe cache an initial probe position here? during FinalizeAction the probe should be in the same position probably?

	if (TimeToPerformAction == 0.f)
	{
		// we'll let the action finalize next frame just so the caller doesn't have to do any special handling for immediate execution
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ATool::OnFinishActionAnimzation));
	}
	else
	{
		FTimerHandle ActionTimerHandle;
		GetWorldTimerManager().SetTimer(ActionTimerHandle, FTimerDelegate::CreateUObject(this, &ATool::OnFinishActionAnimzation), TimeToPerformAction, false);
	}
}

void ATool::OnFinishActionAnimzation()
{
	FinalizeAction();

	OnFinishAction.Execute();
}
