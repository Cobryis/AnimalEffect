// Copyright Epic Games, Inc. All Rights Reserved.

#include "AEPlayerController.h"

#include "AECharacter.h"
#include "AEHUD.h"

AAEPlayerController::AAEPlayerController()
	: APlayerController()
{
	
}


void AAEPlayerController::SetPawn(APawn* NewPawn)
{
	Super::SetPawn(NewPawn);

	TryInitializeHUD();
}

void AAEPlayerController::ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass)
{
	Super::ClientSetHUD_Implementation(NewHUDClass);

	TryInitializeHUD();
}

// #fixme: ideally this would only be called twice per client. seems to be called up to 4 times though. investigate
void AAEPlayerController::TryInitializeHUD()
{
	if (GetPawn() && GetHUD() && (RecastHUD == nullptr))
	{
		RecastHUD = Cast<AAEHUD>(GetHUD());
		GetRecastHUD()->InitializeHUD(FOnFocusChangedDelegate::CreateUObject(this, &AAEPlayerController::OnHUDFocusChanged));
	}
}


void AAEPlayerController::OnHUDFocusChanged(bool bTookFocus)
{
	if (bTookFocus)
	{
		ChangeState(NAME_UI);
	}
	else
	{
		ChangeState(NAME_Playing);
	}
}


void AAEPlayerController::ChangeState(FName NewState)
{
	if (NewState != StateName)
	{
		if (StateName == NAME_UI)
		{
			EndUIState();
		}

		Super::ChangeState(NewState);

		if (StateName == NAME_UI)
		{
			BeginUIState();
		}
	}
}


void AAEPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;

	if (GetPawn())
	{
		GetPawn()->EnableInput(this);
	}
}

void AAEPlayerController::EndPlayingState()
{
	Super::EndPlayingState();
}


void AAEPlayerController::BeginUIState()
{
	if (GetPawn())
	{
		GetPawn()->DisableInput(this);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void AAEPlayerController::EndUIState()
{
	// make sure dialogs are closed if we end UI state prematurely
	if (IsValid(GetRecastHUD()))
	{
		GetRecastHUD()->ForceCloseDialogs();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////////////////

#define stringify(Input) #Input

#define CONFIGURE_STATE_INPUTCOMPONENT(InStateName, StateInputComponent, SetupStateInputComponentFunction)		\
	if (StateName == InStateName)																				\
	{																											\
		if (StateInputComponent == nullptr)																		\
		{																										\
			StateInputComponent = NewObject<UInputComponent>(this, TEXT(stringify(PC_##StateInputComponent)));	\
			SetupStateInputComponentFunction(StateInputComponent);												\
			StateInputComponent->RegisterComponent();															\
			PushInputComponent(StateInputComponent);															\
		}																										\
	}																											\
	else if (StateInputComponent)																				\
	{																											\
		PopInputComponent(StateInputComponent);																	\
		StateInputComponent->DestroyComponent();																\
		StateInputComponent = nullptr;																			\
	}
	


void AAEPlayerController::UpdateStateInputComponents()
{
	Super::UpdateStateInputComponents();

	if (IsLocalController())
	{
		// we can use NAME_UI because it's one of the few replicated FNames
		CONFIGURE_STATE_INPUTCOMPONENT(NAME_UI, UIStateInputComponent, SetupUIStateInputComponent);
		CONFIGURE_STATE_INPUTCOMPONENT(NAME_Playing, PlayingStateInputComponent, SetupPlayingInputComponent);
	}
	
}

//////////////////////////////////////////////////////////////////////////
// Playing Input
//////////////////////////////////////////////////////////////////////////

void AAEPlayerController::SetupPlayingInputComponent(UInputComponent* StateInputComponent)
{
	StateInputComponent->BindAction(TEXT("ToggleInventory"), IE_Pressed, this, &AAEPlayerController::InputAction_ShowInventory);
}


void AAEPlayerController::InputAction_ShowInventory()
{
	if (GetRecastHUD())
	{
		GetRecastHUD()->ShowInventory();
	}
}

//////////////////////////////////////////////////////////////////////////
// UI Input
//////////////////////////////////////////////////////////////////////////

void AAEPlayerController::SetupUIStateInputComponent(UInputComponent* StateInputComponent)
{
	// #todo: is this how we want to handle generic UI things like the back, select, and confirm buttons?

	StateInputComponent->BindAction(TEXT("ToggleInventory"), IE_Pressed, this, &AAEPlayerController::InputAction_HideInventory);
}


void AAEPlayerController::InputAction_HideInventory()
{
	// #todo: make this into an interface for input ? 
	if (GetRecastHUD())
	{
		GetRecastHUD()->HideInventory();
	}
}
