// Copyright Epic Games, Inc. All Rights Reserved.

#include "AECharacter.h"

#include "Data/AEDataAsset.h"
#include "Inventory/InventoryItemInterface.h"
#include "Items/DropActor.h"
#include "Items/Interfaces/InteractableActorInterface.h"
#include "Tools/Tool.h"
#include "WorldGrid/WorldGridInterface.h"
#include "WorldGrid/WorldGridSubsystem.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Gameframework/SpringArmComponent.h"

DECLARE_LOG_CATEGORY_CLASS(LogAECharacter, Log, All);

AAECharacter::AAECharacter(const FObjectInitializer& OI)
	: Super(OI)
{
	CameraArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArmComponent->SetupAttachment(GetRootComponent());
	CameraArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(CameraArmComponent);

	InteractableCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractableCollision"));
	InteractableCollisionComponent->SetupAttachment(GetRootComponent());
	InteractableCollisionComponent->SetCollisionProfileName(TEXT("PawnInteractableCollision"), false);
}

void AAECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, FString::Printf(TEXT("CurrentTool: %s"), IsValid(EquippedToolInstance) ? *EquippedToolInstance->GetName() : TEXT("none")));

	ScanForInteractables();

	TickEquippedTool(DeltaTime);
}

void AAECharacter::TickEquippedTool(float DeltaTime)
{
	if (IsValid(EquippedToolInstance) && !IsPerformingAction())
	{
		EquippedToolInstance->Tick(DeltaTime);
	}
}

bool AAECharacter::IsPerformingAction() const
{
	return bToolIsPerformingAction;
}

void AAECharacter::ScanForInteractables()
{
// 	TArray<AActor*> InteractableActors;
// 	InteractableCollisionComponent->GetOverlappingActors(InteractableActors);

	// see if there's something at our feet to pickup
	{
		CurrentPickup = nullptr; // clear previous scan's pickup

		FGridVector GridPosition;
		if (WorldGrid->GetGridPositionAtWorldLocation(GetActorLocation(), GridPosition))
		{
			// should we be casting to a drop here explicitly? this method is really only for drops
			if (IPickupActorInterface* Pickup = Cast<IPickupActorInterface>(WorldGrid->GetActorAtGridPosition(GridPosition)))
			{
				if (Pickup->CanPickup(this))
				{
					CurrentPickup = Cast<AActor>(Pickup); // #hack
					FText PickupText;
					Pickup->GetPickupText(PickupText);
					GEngine->AddOnScreenDebugMessage(1, 0, FColor::Cyan, FString::Printf(TEXT("Pickup: %s"), *PickupText.ToString()));
				}
			}
		}
	}
}

bool AAECharacter::TryStartInteract()
{
	bWantsToSprint = false; // stop sprinting to start our action?

	// #todo: decide between tool or thing in front of you to interact 

	if (IsValid(EquippedToolInstance) && !IsPerformingAction()) // #note: this will probably need to change for a tool that takes additional action input (like the fishing rod)
	{
		if (EquippedToolInstance->HasAction())
		{
			EquippedToolInstance->StartAction(FOnToolFinishAction::CreateUObject(this, &AAECharacter::OnToolFinishAction));
			bToolIsPerformingAction = true;
			return true;
		}
	}
	return false;
}

void AAECharacter::StopInteract()
{

}

void AAECharacter::OnToolFinishAction()
{
	check(IsValid(EquippedToolInstance));
	bToolIsPerformingAction = false;
}

bool AAECharacter::TryPickup()
{
	if (CurrentPickup)
	{
		FPickupData PickupData;
		CurrentPickup->GetPickupData(PickupData);
		int32 StackSizeRemainingOnGround = TryGiveItemsOfAssetType(PickupData.AssetType, PickupData.StackSize, PickupData.Quality);
		if (StackSizeRemainingOnGround == 0)
		{
			AActor* PickupActor = Cast<AActor>(CurrentPickup.GetObject());
			WorldGrid->RemoveActorFromGrid(PickupActor);
			PickupActor->Destroy();
		}
		else
		{
			AActor* PickupActor = Cast<AActor>(CurrentPickup.GetObject());
		}
	}

	return false;
}

int32 AAECharacter::TryGiveItemsOfAssetType(const UAEMetaAsset* AssetType, int32 Count, uint8 Quality)
{
	int32 CountRemaining = Count;
	// if we're a tool, try adding to the tool inventory first
	if (Cast<IInventoryItemInterface>(AssetType)->GetInventoryType() == EInventoryType::Tool)
	{
		CountRemaining = ToolInventory.TryAdd(AssetType, CountRemaining, Quality);
	}

	if (CountRemaining > 0)
	{
		CountRemaining = ItemInventory.TryAdd(AssetType, CountRemaining, Quality);
	}

	return CountRemaining;
}

void AAECharacter::SwitchToToolInDirection(bool bDirection)
{
	for (int32 i = 1; i < ToolInventory.Size; ++i)
	{
		const int32 SignedIndex = bDirection ? i : -i;
		const int32 CorrectedIndex = (EquippedToolIndex + SignedIndex + ToolInventory.Size) % ToolInventory.Size;
		FInventorySlotData CurrentItem;
		if (ToolInventory.GetAtIndex(CorrectedIndex, CurrentItem))
		{
			EquipTool(CorrectedIndex);
			break;
		}
	}
}

void AAECharacter::EquipTool(int32 ToolIndex)
{
	if (IsPerformingAction())
	{
		return;
	}

	if (EquippedToolIndex != INDEX_NONE)
	{
		UnequipTool(EquippedToolIndex);
	}

	check(EquippedToolIndex == INDEX_NONE);

	FInventorySlotData Tool;
	if (ToolInventory.GetAtIndex(ToolIndex, Tool))
	{
		// #todo: replicate Tool.Class and Call InstanceTool upon replication. 
		EquippedToolInstance = InstanceTool(*Tool.AssetType->GetActorClass());

		if (IsValid(EquippedToolInstance))
		{
			EquippedToolIndex = ToolIndex;

			EquippedToolInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, ToolSocket);
			EquippedToolInstance->OnEquipped();
		}
		else
		{
			UE_LOG(LogAECharacter, Error, TEXT("Failed to equip tool '%s'"), *Tool.AssetType->GetName());
		}
	}
}

void AAECharacter::UnequipTool(int32 ToolIndex)
{
	if (IsPerformingAction())
	{
		return;
	}

	LastEquippedToolIndex = EquippedToolIndex;
	EquippedToolIndex = INDEX_NONE;

	if (IsValid(EquippedToolInstance))
	{
		EquippedToolInstance->OnUnequipped();
		EquippedToolInstance->Destroy();
		EquippedToolInstance = nullptr;
	}
}

namespace
{
	void GenerateSlotHandles(EInventoryType InventoryType, int32 InventorySize, TArray<FInventorySlotHandle>& OutHandles)
	{
		OutHandles.Reset(); // #todo: figure out why we were receiving filled out arrays
		OutHandles.Reserve(InventorySize);
		for (int32 i = 0; i < InventorySize; ++i)
		{
			OutHandles.Add({ InventoryType, i });
		}
	}
}

bool AAECharacter::GetInventoryLayoutData(EInventoryType InventoryType, FInventoryLayoutData& OutData) const
{

	// #note: not using Inventory.Size here because Inventory may not have been initialized yet. 
	// Added a hack to AEPlayerController as well before initializing HUD (which calls this)

	switch (InventoryType)
	{																					
	case EInventoryType::Item:
		OutData.NumColumns = ItemInventory.ColCount;
		OutData.NumRows = ItemInventory.RowCount;
		GenerateSlotHandles(InventoryType, ItemInventory.ColCount * ItemInventory.RowCount, OutData.SlotHandles);
		return true;
	case EInventoryType::Tool:
		OutData.NumColumns = ToolInventory.ColCount;
		OutData.NumRows = ToolInventory.RowCount;
		GenerateSlotHandles(InventoryType, ItemInventory.ColCount * ItemInventory.RowCount, OutData.SlotHandles);
		return true;
	default:
		return false;
	}
}

FInventory& AAECharacter::GetInventoryFromHandle(const FInventorySlotHandle& Handle)
{
	switch (Handle.InventoryType)
	{
	case EInventoryType::Item:
		return ItemInventory;
	case EInventoryType::Tool:
		return ToolInventory;
	default:
		check(false);
	}

	return ItemInventory;
}

const FInventory& AAECharacter::GetInventoryFromHandle(const FInventorySlotHandle& Handle) const
{
	switch (Handle.InventoryType)
	{
	case EInventoryType::Item:
		return ItemInventory;
	case EInventoryType::Tool:
		return ToolInventory;
	default:
		check(false);
	}

	return ItemInventory;
}

bool AAECharacter::GetInventorySlotFromHandle(const FInventorySlotHandle& Handle, FInventorySlotData& OutData) const
{
	return GetInventoryFromHandle(Handle).GetAtIndex(Handle.SlotIndex, OutData);
}

bool AAECharacter::GetInventorySlotActionsFromHandle(const FInventorySlotHandle& Handle, TArray<EInventoryItemActions>& Actions) const
{
	FInventorySlotData SlotData;
	if (GetInventorySlotFromHandle(Handle, SlotData))
	{
		return Cast<IInventoryItemInterface>(SlotData.AssetType)->GetAvailableItemActions(Actions);
	}

	return false;
}

void AAECharacter::OnInventorySlotActionSelectedForHandle(const FInventorySlotHandle& Handle, EInventoryItemActions Action)
{
#if !UE_BUILD_SHIPPING
	FInventorySlotData SlotData;
	if (GetInventorySlotFromHandle(Handle, SlotData))
	{
		TArray<EInventoryItemActions> AvailableActions;
		Cast<IInventoryItemInterface>(SlotData.AssetType)->GetAvailableItemActions(AvailableActions);

		if (!AvailableActions.Contains(Action))
		{
			ensure(false);
			UE_LOG(LogAECharacter, Error, TEXT("Attempted to perform action '%s' that does not exist on class '%s'"), *UEnum::GetValueAsString(Action), *SlotData.AssetType->GetName());
			return;
		}
	}
	else
	{
		ensure(false);
		UE_LOG(LogAECharacter, Error, TEXT("Attempted to perform action '%s' on empty slot at index: %d"), *UEnum::GetValueAsString(Action), Handle.SlotIndex);
		return;
	}
#endif // !UE_BUILD_SHIPPING

	switch (Action)
	{
	case EInventoryItemActions::Drop:
		TryDropItemForSlotHandle(Handle);
		break;
	case EInventoryItemActions::Hold:
		TryHoldItemForSlotHandle(Handle);
		break;
	case EInventoryItemActions::Place:
		TryPlaceItemForSlotHandle(Handle);
		break;
	default:
		ensure(false);
		UE_LOG(LogAECharacter, Error, TEXT("Attempted to perform unsupported action '%s'"), *UEnum::GetValueAsString(Action));
		return;
	}
}

bool AAECharacter::TryDropItemForSlotHandle(const FInventorySlotHandle& Handle)
{
	check(!IsPerformingAction()); // this callback should really only come from a menu, during which we should not be in the state "IsPerformingAction"

	if (Handle.InventoryType == EInventoryType::Tool && Handle.SlotIndex == EquippedToolIndex)
	{
		EquipTool(INDEX_NONE);
	}

	FInventorySlotData RemovedItem;
	if (GetInventoryFromHandle(Handle).TryRemoveAtIndex(Handle.SlotIndex, RemovedItem))
	{
		FGridVector DropPosition;
		if (WorldGrid->GetGridPositionAtWorldLocation(GetActorLocation(), DropPosition))
		{
			if (WorldGrid->GetVacantPositionAtOrNearPosition(DropPosition, FGridVector(1, 1), DropPosition))
			{
				FPickupData PickupData;
				PickupData.AssetType = RemovedItem.AssetType;
				PickupData.StackSize = RemovedItem.StackSize;
				PickupData.Quality = RemovedItem.Quality;
				return IsValid(ADropActor::NewDrop(this, DropPosition, PickupData, this));
			}
		}

		// #todo: message system
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("No room to drop item."));
	}
	return false;
}

bool AAECharacter::TryPlaceItemForSlotHandle(const FInventorySlotHandle& Handle)
{
	check(!IsPerformingAction()); // this callback should really only come from a menu, during which we should not be in the state "IsPerformingAction"

	if (Handle.InventoryType == EInventoryType::Tool && Handle.SlotIndex == EquippedToolIndex)
	{
		EquipTool(INDEX_NONE);
	}

	FInventorySlotData ItemSlot;
	if (GetInventoryFromHandle(Handle).GetAtIndex(Handle.SlotIndex, ItemSlot))
	{
		// #hack: we should not have to get worldgridinterface here
		const IWorldGridInterface* WorldGridInterface = Cast<IWorldGridInterface>(ItemSlot.AssetType);
		const FGridVector ActorSize = WorldGridInterface->GetWorldGridSize();

		const FVector TestLocation = GetActorLocation() + GetActorRotation().Vector() * static_cast<float>(FMath::Max(ActorSize.X, ActorSize.Y)) * 100.f;

		FGridVector PlacePosition;
		if (WorldGrid->GetGridPositionAtWorldLocation(TestLocation, PlacePosition))
		{
			WorldGrid->DebugDrawPosition(PlacePosition, 5.f, FColor::Blue);

			FWorldGridActorSpawnParameters SpawnParams;
			SpawnParams.bCanAdjustPosition = true;
			SpawnParams.DesiredPosition = PlacePosition;
			SpawnParams.Owner = this;
			if (AActor* PlacingActor = WorldGrid->TrySpawnActorOnGrid(ItemSlot.AssetType, SpawnParams))
			{
				GetInventoryFromHandle(Handle).TryRemoveSingleAtIndex(Handle.SlotIndex, ItemSlot);
				return true;
			}
		}
		else
		{
			UE_LOG(LogAECharacter, Error, TEXT("Failed to find valid GridPosition during TryPlaceItem"));
		}
	}

	return false;
}

bool AAECharacter::TryHoldItemForSlotHandle(const FInventorySlotHandle& Handle)
{
	FInventorySlotData CurrentItem;
	if (ToolInventory.GetAtIndex(Handle.SlotIndex, CurrentItem))
	{
		EquipTool(Handle.SlotIndex);
	}

	return true;
}

void AAECharacter::BeginPlay()
{
	Super::BeginPlay();

	ToolInventory.InitializeInventory(this, EInventoryType::Tool, true);
	ItemInventory.InitializeInventory(this, EInventoryType::Item);

	WorldGrid = GetWorld()->GetSubsystem<UWorldGridSubsystem>();
}

void AAECharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AAECharacter::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AAECharacter::InputAxis_MoveRight);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AAECharacter::InputAction_SprintPressed);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AAECharacter::InputAction_SprintReleased);

	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AAECharacter::InputAction_InteractPressed);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Released, this, &AAECharacter::InputAction_InteractReleased);

	PlayerInputComponent->BindAction(TEXT("Pickup"), IE_Pressed, this, &AAECharacter::InputAction_PickupPressed);

	PlayerInputComponent->BindAction(TEXT("EquipNextTool"), IE_Pressed, this, &AAECharacter::InputAction_EquipNextTool);
	PlayerInputComponent->BindAction(TEXT("EquipPrecedingTool"), IE_Pressed, this, &AAECharacter::InputAction_EquipPrecedingTool);
	PlayerInputComponent->BindAction(TEXT("EquipPreviousTool"), IE_Pressed, this, &AAECharacter::InputAction_EquipPreviousTool);
}

ATool* AAECharacter::InstanceTool(TSubclassOf<ATool> ToolClass)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.bDeferConstruction = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Template = ToolClass.GetDefaultObject();

	ATool* InstancedTool = GetWorld()->SpawnActor<ATool>(ToolClass, FTransform::Identity, SpawnParams);
	InstancedTool->FinishSpawning(FTransform::Identity);

	return InstancedTool;
}

//////////////////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////////////////

void AAECharacter::InputAxis_MoveForward(float Value)
{
	if (!IsPerformingAction())
	{
		AddMovementInput(FVector::RightVector, FMath::Clamp(-Value, -1.f, 1.f));
	}
}

void AAECharacter::InputAxis_MoveRight(float Value)
{
	if (!IsPerformingAction())
	{
		AddMovementInput(FVector::ForwardVector, FMath::Clamp(Value, -1.f, 1.f));
	}
}

void AAECharacter::InputAction_SprintPressed()
{
	if (!IsPerformingAction())
	{
		bWantsToSprint = true;
	}
}

void AAECharacter::InputAction_SprintReleased()
{
	bWantsToSprint = false;
}

void AAECharacter::InputAction_InteractPressed()
{
	TryStartInteract();
}

void AAECharacter::InputAction_InteractReleased()
{
	StopInteract();
}

void AAECharacter::InputAction_PickupPressed()
{
	if (!IsPerformingAction())
	{
		TryPickup();
	}
}

void AAECharacter::InputAction_EquipNextTool()
{
	if (!IsPerformingAction())
	{
		SwitchToToolInDirection(true);
	}
}

void AAECharacter::InputAction_EquipPrecedingTool()
{
	if (!IsPerformingAction())
	{
		SwitchToToolInDirection(false);
	}
}

void AAECharacter::InputAction_EquipPreviousTool()
{
	if (!IsPerformingAction())
	{
		EquipTool(LastEquippedToolIndex);
	}
}