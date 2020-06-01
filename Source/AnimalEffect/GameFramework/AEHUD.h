// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Gameframework/HUD.h"

#include "AEHUD.generated.h"

class UFocusDialog;

DECLARE_DELEGATE_OneParam(FOnFocusChangedDelegate, bool);

UCLASS()
class ANIMALEFFECT_API AAEHUD : public AHUD
{
	GENERATED_BODY()

public:

	AAEHUD();

	void InitializeHUD(const FOnFocusChangedDelegate& InFocusChangedDelegate);

	void ShowHUD() override;

	void ShowInventory();
	void HideInventory();

	void ForceCloseDialogs();

protected:

	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:

	void PushFocusRequest(UFocusDialog* Dialog);
	void PopFocusRequest(UFocusDialog* Dialog);

private:

	UPROPERTY(Transient)
	TArray<UFocusDialog*> FocusDialogStack;

	FOnFocusChangedDelegate OnFocusChangedDelegate;

	TSubclassOf<UUserWidget> BlueprintHUDWidgetClass = nullptr;

	UPROPERTY(Transient)
	UUserWidget* BlueprintHUDWidget;

	TSubclassOf<UFocusDialog> MainInventoryWidgetClass = nullptr;

	UPROPERTY(Transient)
	UFocusDialog* MainInventoryWidget;

};
