// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Items/ItemAsset.h"
#include "WorldGrid/WorldGridTypes.h"

#include "GameFramework/Actor.h"

#include "Tool.generated.h"

UCLASS()
class ANIMALEFFECT_API UToolAsset : public UItemAsset
{
	GENERATED_BODY()

public:

	UToolAsset();

};

class USkeletalMeshComponent;
class UWorldGridSubsystem;

DECLARE_DELEGATE(FOnToolFinishAction);

// a tool shouldn't be activatable if the character is doing something other than simply holding the tool
// a tool shouldn't tick if the character is doing something other than simply holding the tool
// then maybe actor tick should just directly drive tool tick
UCLASS(Abstract)
class ANIMALEFFECT_API ATool : public AActor
{
	GENERATED_BODY()

public:

	ATool();

	virtual void OnEquipped();
	virtual void OnUnequipped();

	virtual bool HasAction() const { return true; }

	virtual void StartAction(const FOnToolFinishAction& OnFinishActionCallback);

	FORCEINLINE FVector GetProbeLocation() const { return GetOwner()->GetActorLocation() + GetOwner()->GetActorRotation().Vector() * ProbeLength; }

protected:

	FORCEINLINE UWorldGridSubsystem* GetWorldGrid() const { return WorldGrid; }

	virtual void FinalizeAction() PURE_VIRTUAL(ATool::FinalizeAction, );

private:

	void OnFinishActionAnimzation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess=true))
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Tool")
	float ProbeLength = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tool")
	float TimeToPerformAction = 0.f;

	FOnToolFinishAction OnFinishAction;

	UPROPERTY(Transient)
	UWorldGridSubsystem* WorldGrid;

};
