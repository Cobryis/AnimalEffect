// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridTypes.h"

#include "WorldGridInterface.generated.h"

UINTERFACE()
class ANIMALEFFECT_API UWorldGridActorInterface : public UInterface
{
	GENERATED_BODY()
};

// this is an "Actor" interface for now so it belongs on an actor and not a data asset
class ANIMALEFFECT_API IWorldGridActorInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual void GetWorldGridSize(int32& X, int32& Y) const = 0;

	// the actor's world grid pivot point should be near 0,0 in grid space
	// #todo: it might be kinda dumb that the actor owns their grid position.
	// it could be changed accidentally and not match up with WorldGridInterface anymore.
	// we may want to switch to using UObjectAnnotation instead to track this data.
	virtual void SetWorldGridPosition(const FGridPosition& GridPosition) = 0;
	virtual void GetWorldGridPosition(FGridPosition& GridPosition) const = 0;

};
