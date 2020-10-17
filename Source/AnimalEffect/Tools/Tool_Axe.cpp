// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tool_Axe.h"

#include "Environment/Tree.h"
#include "WorldGrid/WorldGridSubsystem.h"

ATool_Axe::ATool_Axe()
	: ATool()
{

}

void ATool_Axe::FinalizeAction()
{
	FGridVector ProbePosition;
	if (GetWorldGrid()->GetGridPositionAtWorldLocation(GetProbeLocation(), ProbePosition))
	{
		AActor* HitActor = GetWorldGrid()->GetActorAtGridPosition(ProbePosition);
		if (ATree* HitTree = Cast<ATree>(HitActor))
		{
			HitTree->OnAxeHit(GetInstigator());
		}
	}
}

