// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/Inventory.h"
#include "Inventory/InventoryAccessInterface.h"
#include "Items/Interfaces/PickupActorInterface.h"

#include "Gameframework/Character.h"

#include "AECharacter.generated.h"

class AItem;
class ATool;
class UBoxComponent;
class UCameraComponent;
class USphereComponent;
class USpringArmComponent;
class UInputComponent;
class UWorldGridSubsystem;

UCLASS()
class ANIMALEFFECT_API AAECharacter : public ACharacter, public IInventoryAccessInterface
{
	GENERATED_BODY()

public:

	AAECharacter(const FObjectInitializer& OI);

	void Tick(float DeltaTime) override;

	void TickEquippedTool(float DeltaTime);

	bool IsPerformingAction() const;

	bool TryStartInteract();
	void StopInteract();

	bool TryPickup();

	virtual int32 TryGiveItemsOfAssetType(const UAEMetaAsset* AssetType, int32 Count, uint8 Quality);

	void EquipTool(int32 ToolIndex);
	void UnequipTool(int32 ToolIndex);

	FORCEINLINE FInventory& GetItemInventory() { return ItemInventory; }
	FORCEINLINE const FInventory& GetItemInventory() const { return ItemInventory; }

	FORCEINLINE FInventory& GetToolInventory() { return ToolInventory; }
	FORCEINLINE const FInventory& GetToolInventory() const { return ToolInventory; }

	// BEGIN IInventoryAccessInterface
	UFUNCTION()
	bool GetInventoryLayoutData(EInventoryType InventoryType, FInventoryLayoutData& OutData) const override;

	FInventory& GetInventoryFromHandle(const FInventorySlotHandle& Handle);
	const FInventory& GetInventoryFromHandle(const FInventorySlotHandle& Handle) const;

	UFUNCTION()
	bool GetInventorySlotFromHandle(const FInventorySlotHandle& Handle, FInventorySlotData& OutData) const override;

	UFUNCTION()
	bool GetInventorySlotActionsFromHandle(const FInventorySlotHandle& Handle, TArray<EInventoryItemActions>& Actions) const override;

	UFUNCTION()
	void OnInventorySlotActionSelectedForHandle(const FInventorySlotHandle& Handle, EInventoryItemActions Action) override;
	// END IInventoryAccessInterface

	bool TryDropItemForSlotHandle(const FInventorySlotHandle& Handle);
	bool TryPlaceItemForSlotHandle(const FInventorySlotHandle& Handle);
	bool TryHoldItemForSlotHandle(const FInventorySlotHandle& Handle);

protected:

	void BeginPlay() override;

	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	ATool* InstanceTool(TSubclassOf<ATool> ToolClass);

	void ScanForInteractables();

private:

	void OnToolFinishAction();

	void SwitchToToolInDirection(bool bDirection);

private:

	void InputAxis_MoveForward(float Value);
	void InputAxis_MoveRight(float Value);

	void InputAction_SprintPressed();
	void InputAction_SprintReleased();

	void InputAction_InteractPressed();
	void InputAction_InteractReleased();
	void InputAction_PickupPressed();

	void InputAction_EquipNextTool();
	void InputAction_EquipPrecedingTool();
	void InputAction_EquipPreviousTool();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess=true))
	USpringArmComponent* CameraArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess=true))
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess=true))
	UBoxComponent* InteractableCollisionComponent;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FInventory ItemInventory;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FInventory ToolInventory;

	UPROPERTY(EditDefaultsOnly, Category = "Tool")
	FName ToolSocket = TEXT("tool_r");

	UPROPERTY(Transient)
	int32 EquippedToolIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 LastEquippedToolIndex = INDEX_NONE;

	UPROPERTY(Transient)
	ATool* EquippedToolInstance;

	UPROPERTY(Transient)
	bool bToolIsPerformingAction;

	UPROPERTY(Transient)
	UWorldGridSubsystem* WorldGrid;

	UPROPERTY(Transient)
	TScriptInterface<IPickupActorInterface> CurrentPickup;

	UPROPERTY(Transient)
	bool bWantsToSprint;

};
