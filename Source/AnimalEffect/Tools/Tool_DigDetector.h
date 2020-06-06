// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tool.h"

#include "Tool_DigDetector.generated.h"

// #todo: move me to my own file
UCLASS()
class ANIMALEFFECT_API ATool_MetalDetector : public ATool
{
	GENERATED_BODY()

public:

	ATool_MetalDetector();

	void Tick(float DeltaSeconds) override;

	void FinalizeAction() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* DetectionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Marker")
	TSubclassOf<AGridMarker> MarkerClass;

	bool bIsDetectorActive = true;

	float LastBeepTime = 0.f;

	TOptional<FGridVector> ProbePositionCache;

};

UCLASS()
class ANIMALEFFECT_API AGridMarker : public AActor
{
	GENERATED_BODY()

public:

	AGridMarker();

	static AGridMarker* NewMarker(const UObject* WorldContextObject, TSubclassOf<AGridMarker> MarkerClass, const FGridVector& SpawnPosition, APawn* Instigator);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess=true))
	UStaticMeshComponent* MeshComponent;

};