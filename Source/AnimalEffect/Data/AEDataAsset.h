// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "UObject/UObjectAnnotation.h"

#include "AEDataAsset.generated.h"

UCLASS()
class ANIMALEFFECT_API UAEMetaAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

private:

	UPROPERTY(EditDefaultsOnly, Category = "Actor")
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FText Title;

public:

	TSubclassOf<AActor> GetActorClass() const;

	static const UAEMetaAsset* GetMetaAssetForClass(UClass* Class);

	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetTitle() const;

};
