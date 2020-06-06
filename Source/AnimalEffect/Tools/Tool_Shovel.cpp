// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tool_Shovel.h"

#include "Tool_DigDetector.h"
#include "Data/DigActualizer.h"
#include "GameFramework/AECharacter.h"
#include "WorldGrid/WorldGridSubsystem.h"

ATool_Shovel::ATool_Shovel()
	: ATool()
{

}

void ATool_Shovel::FinalizeAction()
{
	UWorldGridSubsystem* WGS = GetWorld()->GetSubsystem<UWorldGridSubsystem>();

	FGridVector ProbePosition;
	if (WGS->GetGridPositionAtWorldLocation(GetProbeLocation(), ProbePosition))
	{
		AActor* BlockingActor = WGS->GetActorAtGridPosition(ProbePosition);

		if (IsValid(BlockingActor) && BlockingActor->IsA<AGridMarker>())
		{
			WGS->RemoveActorFromGrid(BlockingActor);
			BlockingActor->Destroy();
			BlockingActor = nullptr;
		}
		
		if (BlockingActor == nullptr)
		{
			TSoftObjectPtr<UDigActualizer> DigActualizer = WGS->TryRemoveDigActualizerFromGrid(ProbePosition);
			DigActualizer.LoadSynchronous(); // #fixme?
			if (DigActualizer.IsValid())
			{
				AAECharacter* OwnerCharacter = Cast<AAECharacter>(GetOwner());
				FPickupData DigPickup = DigActualizer->Actualize();
				OwnerCharacter->TryGiveItemsOfAssetType(DigPickup.AssetType, DigPickup.StackSize, DigPickup.Quality);
			}
		}

		WGS->DebugDrawPosition(ProbePosition, 2.f, FColor::Red);
	}
}
