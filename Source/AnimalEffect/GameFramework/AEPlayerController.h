// Copyright Epic Games, Inc. All Rights Reserved.

#pragma

#include "Gameframework/PlayerController.h"

#include "AEPlayerController.generated.h"

class AAEHUD;

UCLASS()
class ANIMALEFFECT_API AAEPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AAEPlayerController();

	void SetPawn(APawn* NewPawn) override;
	void ChangeState(FName NewState) override;

	FORCEINLINE AAEHUD* GetRecastHUD() const { return RecastHUD; }

private:

	void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

	void TryInitializeHUD();

	void OnHUDFocusChanged(bool bTookFocus);

	void BeginPlayingState() override;
	void EndPlayingState() override;

	void BeginUIState();
	void EndUIState();

// Input

	void UpdateStateInputComponents() override;

	void SetupPlayingInputComponent(UInputComponent* StateInputComponent);
	void InputAction_ShowInventory();

	void SetupUIStateInputComponent(UInputComponent* StateInputComponent);
	void InputAction_HideInventory();

private:

	UPROPERTY()
	UInputComponent* UIStateInputComponent;

	UPROPERTY()
	UInputComponent* PlayingStateInputComponent;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AAEHUD* RecastHUD;

};
