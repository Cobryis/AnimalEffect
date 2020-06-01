// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Items/Item.h"

#include "Tool.generated.h"

UCLASS()
class ANIMALEFFECT_API UToolAsset : public UItemAsset
{
	GENERATED_BODY()

public:

	UToolAsset();

};

class USkeletalMeshComponent;

UCLASS(Abstract)
class ANIMALEFFECT_API ATool : public AItem
{
	GENERATED_BODY()

public:

	ATool();

	virtual void OnEquipped();
	virtual void OnUnequipped();

	virtual void OnActivate();

	FORCEINLINE FVector GetProbeLocation() const { return GetOwner()->GetActorLocation() + GetOwner()->GetActorRotation().Vector() * ProbeLength; }

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess=true))
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Tool")
	float ProbeLength = 80.f;

};
