// Copyright Epic Games, Inc. All Rights Reserved.

#include "AEHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/FocusDialog.h"

AAEHUD::AAEHUD()
	: AHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BlueprintHUDFinder(TEXT("/Game/UI/HUD/WBP_HUD"));
	if (BlueprintHUDFinder.Succeeded())
	{
		BlueprintHUDWidgetClass = BlueprintHUDFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> MainInventoryFinder(TEXT("/Game/UI/HUD/WBP_ToolAndItemInventory"));
	if (MainInventoryFinder.Succeeded())
	{
		MainInventoryWidgetClass = MainInventoryFinder.Class;
	}
}

void AAEHUD::InitializeHUD(const FOnFocusChangedDelegate& InFocusChangedDelegate)
{
	OnFocusChangedDelegate = InFocusChangedDelegate;

	BlueprintHUDWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), BlueprintHUDWidgetClass, TEXT("BlueprintHUDWidget"));
	BlueprintHUDWidget->AddToPlayerScreen(0);

	MainInventoryWidget = CreateWidget<UFocusDialog>(GetOwningPlayerController(), MainInventoryWidgetClass, TEXT("MainInventoryWidget"));
	MainInventoryWidget->InitializeFocusDialog(FOnCloseDialogDelegate::CreateUObject(this, &AAEHUD::PopFocusRequest), 1, true);
}

void AAEHUD::ShowHUD()
{
	Super::ShowHUD();

	// #todo: hide UUserWidgets.. account for MainInventoryWidget Show/Hide input
}

void AAEHUD::ShowInventory()
{
	if (IsValid(MainInventoryWidget))
	{
		PushFocusRequest(MainInventoryWidget);
		MainInventoryWidget->Show();
	}
}

void AAEHUD::HideInventory()
{
	if (IsValid(MainInventoryWidget))
	{
		// Pop is handled in the OnCloseDialogDelegate
		MainInventoryWidget->Close();
	}
}

void AAEHUD::ForceCloseDialogs()
{
	TArray<UFocusDialog*> FocusStackCopy = FocusDialogStack;
	for (UFocusDialog* Dialog : FocusStackCopy)
	{
		Dialog->Close();
	}
}

void AAEHUD::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	ForceCloseDialogs();
}

void AAEHUD::PushFocusRequest(UFocusDialog* Dialog)
{
	check(!FocusDialogStack.Contains(Dialog));
	FocusDialogStack.Push(Dialog);

	// focus gained on first push
	if (FocusDialogStack.Num() == 1)
	{
		OnFocusChangedDelegate.Execute(true);
	}
}

void AAEHUD::PopFocusRequest(UFocusDialog* Dialog)
{
	const int32 DialogRemoved = FocusDialogStack.RemoveSingle(Dialog);

	check(DialogRemoved == 1); // should never try to pop something that wasn't pushed

	if (FocusDialogStack.Num() == 0)
	{
		OnFocusChangedDelegate.Execute(false);
	}
}
