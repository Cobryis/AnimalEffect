// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridTypes.h"

#include "WorldGridInterface.generated.h"

UINTERFACE()
class ANIMALEFFECT_API UWorldGridInterface : public UInterface
{
	GENERATED_BODY()
};

// this is an "Actor" interface for now so it belongs on an actor and not a data asset
class ANIMALEFFECT_API IWorldGridInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual FGridVector GetWorldGridSize() const = 0;

};
