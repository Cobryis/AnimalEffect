// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropActor.h"

#include "Data/AEDataAsset.h"
#include "Interfaces/DropInterface.h"
#include "WorldGrid/WorldGridSubsystem.h"

#include "Components/BillboardComponent.h"

DECLARE_LOG_CATEGORY_CLASS(LogDrop, Log, All)

ADropActor::ADropActor()
	: AActor()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

ADropActor* ADropActor::NewDrop(const UObject* WorldContextObject, const FGridPosition& SpawnPosition, const FPickupData& PickupData, APawn* Instigator)
{
	if (PickupData.AssetType == nullptr)
	{
		UE_LOG(LogDrop, Error, TEXT("Attempted to spawn drop with invalid asset type"));
		return nullptr;
	}

	if (!PickupData.AssetType->Implements<UDropInterface>())
	{
		UE_LOG(LogDrop, Error, TEXT("Attempted to spawn drop with asset type '%s' that doesn't implement DropInterface"), *PickupData.AssetType->GetName());
		return nullptr;
	}

	if (PickupData.StackSize == 0)
	{
		UE_LOG(LogDrop, Error, TEXT("Attempted to spawn drop with 0 stack size"));
		return nullptr;
	}

	if (PickupData.Quality == 0)
	{
		UE_LOG(LogDrop, Error, TEXT("Attempted to spawn drop with 0 quality"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);

	UWorldGridSubsystem* const WGS = World->GetSubsystem<UWorldGridSubsystem>();
	check(WGS);

	if (!WGS->IsValidPosition(SpawnPosition))
	{
		UE_LOG(LogDrop, Warning, TEXT("Attempted to spawn drop at invalid position: %s"), *SpawnPosition.ToString());
		return nullptr;
	}

	const IDropInterface* DropInterface = Cast<IDropInterface>(PickupData.AssetType);

	static int32 DropCount = 0;

	FActorSpawnParameters SpawnParams;
	SpawnParams.bDeferConstruction = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = Instigator;
	SpawnParams.Name = *FString::Printf(TEXT("DropActor_%s_%d"), *PickupData.AssetType->GetName(), DropCount);

	DropCount++;

	ADropActor* DropActor = World->SpawnActor<ADropActor>(SpawnParams);
	DropActor->MeshComponent->SetStaticMesh(DropInterface->GetDropMesh());
	DropActor->DropText = DropInterface->GetDropDescription();
	DropActor->PickupData = PickupData;

	// #todo: this part feels a bit rough. maybe we shouldn't be spawning the actor till we know we can place it
	FGridPosition FinalPosition;
	if (WGS->TryPlaceActorOnGrid(DropActor, SpawnPosition, true, FinalPosition))
	{
		FVector WorldLocation;
		WGS->GetWorldLocationCenteredAtGridPosition(FinalPosition, WorldLocation);
		DropActor->FinishSpawning(FTransform(WorldLocation));
	}
	else
	{
		UE_LOG(LogDrop, Warning, TEXT("Failed to place DropActor on grid. Destroying DropActor."));
		DropActor->Destroy();
		DropActor = nullptr;
	}

	return DropActor;
}

void ADropActor::SetWorldGridPosition(const FGridPosition& InGridPosition)
{
	GridPosition = InGridPosition;
}

void ADropActor::GetWorldGridPosition(FGridPosition& OutGridPosition) const
{
	OutGridPosition = GridPosition;
}

ADropSpawner::ADropSpawner()
	: AActor()
{
	UBillboardComponent* IconComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	// #todo: set icon texture in editor
	SetRootComponent(IconComponent);
}

void ADropSpawner::BeginPlay()
{
	Super::BeginPlay();

	// #todo: eventually we should show the drop in editor before begin play

	const UWorldGridSubsystem* const WGS = GetWorld()->GetSubsystem<UWorldGridSubsystem>();

	FGridPosition DesiredPosition;
	if (WGS->GetGridPositionAtWorldLocation(GetActorLocation(), DesiredPosition))
	{
		ADropActor::NewDrop(this, DesiredPosition, PickupData, nullptr);
	}
	else
	{
		UE_LOG(LogDrop, Warning, TEXT("Drop spawner '%s' cannot find position on grid to spawn drop"), *GetName());
	}
}
