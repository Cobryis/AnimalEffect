// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"

#include "PickupActorInterface.generated.h"

class UAEMetaAsset;

USTRUCT(BlueprintType)
struct ANIMALEFFECT_API FPickupData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	const UAEMetaAsset* AssetType; 

	UPROPERTY(EditAnywhere)
	uint8 StackSize = 1;

	UPROPERTY(EditAnywhere)
	uint8 Quality = 1;

	FORCEINLINE bool IsValid() const { return AssetType != nullptr; }

};

UINTERFACE()
class ANIMALEFFECT_API UPickupActorInterface : public UInterface
{
	GENERATED_BODY()
};

class ANIMALEFFECT_API IPickupActorInterface : public IInterface
{
	GENERATED_BODY()

public:

	virtual bool CanPickup(APawn* Instigator) const = 0;
	virtual void GetPickupText(FText& OutPickupText) const { OutPickupText = FText::GetEmpty(); }
	virtual void GetPickupData(FPickupData& OutPickupData) const = 0;
	virtual void UpdatePickupStackSize(uint8 NewStackSize) { check(false); }
	virtual void UpdatePickupQuality(uint8 NewQuality) { check(false); }

};
