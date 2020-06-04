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

ADropActor* ADropActor::NewDrop(const UObject* WorldContextObject, const FGridVector& SpawnPosition, const FPickupData& PickupData, APawn* Instigator)
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

	FWorldGridActorSpawnParameters SpawnParams;
	SpawnParams.bCanAdjustPosition = true;
	SpawnParams.DesiredPosition = SpawnPosition;
	SpawnParams.Owner = Instigator;

	return Cast<ADropActor>
	(
		UWorldGridSubsystem::Get(WorldContextObject)->TrySpawnSmallActorOnGrid(ADropActor::StaticClass(), SpawnParams,
			[&PickupData, Instigator](AActor* SpawningActor)
			{
				auto DropActor = static_cast<ADropActor*>(SpawningActor);
				const IDropInterface* DropInterface = Cast<IDropInterface>(PickupData.AssetType);
				DropActor->MeshComponent->SetStaticMesh(DropInterface->GetDropMesh());
				DropActor->DropText = DropInterface->GetDropDescription();
				DropActor->PickupData = PickupData;
			})
	);
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

	FGridVector DesiredPosition;
	if (WGS->GetGridPositionAtWorldLocation(GetActorLocation(), DesiredPosition))
	{
		ADropActor::NewDrop(this, DesiredPosition, PickupData, nullptr);
	}
	else
	{
		UE_LOG(LogDrop, Warning, TEXT("Drop spawner '%s' cannot find position on grid to spawn drop"), *GetName());
	}
}
