// Copyright Epic Games, Inc. All Rights Reserved.

#pragma

#include "WorldGrid/WorldGridSubsystem.h"

#include "GameFramework/WorldSettings.h"

#include "AEWorldSettings.generated.h"

UCLASS()
class AAEWorldSettings : public AWorldSettings, public IWorldGridConfigInterface
{
	GENERATED_BODY()

public:

	AAEWorldSettings();

	void GetWorldGridConfig(FWorldGridConfig& OutConfig) override;

private:
	
	UPROPERTY(EditAnywhere, Category = "World Grid")
	FWorldGridConfig WorldGrid;

};