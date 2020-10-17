// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tool.h"

#include "Tool_Axe.generated.h"

UCLASS()
class ANIMALEFFECT_API ATool_Axe : public ATool
{
	GENERATED_BODY()

public:

	ATool_Axe();

	void FinalizeAction() override;

};
