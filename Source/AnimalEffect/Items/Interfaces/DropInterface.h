// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DropInterface.generated.h"

UINTERFACE(BlueprintType)
class ANIMALEFFECT_API UDropInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IDropInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual UStaticMesh* GetDropMesh() const = 0;

	virtual FText GetDropDescription() const = 0;

};
