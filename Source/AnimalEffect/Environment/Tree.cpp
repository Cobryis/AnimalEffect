// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tree.h"

#include "WorldGrid/WorldGridSubsystem.h"

ATree::ATree()
	: AActor()
{

}

void ATree::OnAxeHit(APawn* HitInstigator)
{
	if (Health == 0)
	{
		return;
	}

	Health--;

	if (Health == 0)
	{
		
	}
}
