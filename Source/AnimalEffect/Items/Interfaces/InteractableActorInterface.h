// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InteractableActorInterface.generated.h"


UINTERFACE()
class ANIMALEFFECT_API UInteractableActorInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IInteractableActorInterface : public IInterface
{
	GENERATED_BODY()

	virtual void Interact(APawn* InteractInstigator) const = 0;

	// #todo: figure out these functions
	virtual void Push() const {}
	virtual void Pull() const {}
	virtual void RotateClockwise() const {}
	virtual void RotateCounterClockwise() const {}

};
