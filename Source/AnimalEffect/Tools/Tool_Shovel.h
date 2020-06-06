// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tool.h"

#include "Tool_Shovel.generated.h"

UCLASS()
class ANIMALEFFECT_API ATool_Shovel : public ATool
{
	GENERATED_BODY()

public:

	ATool_Shovel();

	void FinalizeAction() override;

};
