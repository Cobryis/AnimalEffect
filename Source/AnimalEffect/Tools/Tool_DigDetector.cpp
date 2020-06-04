// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tool_DigDetector.h"

#include "WorldGrid/WorldGridSubsystem.h"

#include "Kismet/GameplayStatics.h"

ATool_MetalDetector::ATool_MetalDetector()
	: ATool()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = .1f;
}

void ATool_MetalDetector::Tick(float DeltaSeconds)
{
	ProbePositionCache.Reset();

	if (bIsDetectorActive)
	{
		UWorldGridSubsystem* WGS = GetWorld()->GetSubsystem<UWorldGridSubsystem>();

		FGridVector ProbePosition;
		if (WGS->GetGridPositionAtWorldLocation(GetProbeLocation(), ProbePosition))
		{
			ProbePositionCache = ProbePosition;

			LastBeepTime += DeltaSeconds;

			// first value is rarity, second is distance. where 0 is closest to target
			TTuple<int32, int32> DetectionData = WGS->GetDetectionDataAtPosition(ProbePosition);
			// rarity must be greater than 1
			const int32 Rarity = DetectionData.Get<0>();
			if (Rarity > 0)
			{
				const float MaxDist = 20.f;

				const int32 Distance = DetectionData.Get<1>();
				const float ToneVolume = FMath::Square(FMath::GetMappedRangeValueClamped({ 0, MaxDist }, { 1.f, .1f }, static_cast<float>(Distance)));
				const float TonePitch = FMath::GetMappedRangeValueClamped({ 1, 10 }, { .5f, 2.f }, static_cast<float>(Rarity));

				const TArray<float> ToneFrequencyMap = { 1.2f, 1.1f, 1.f, .9f, .8f, .7f, .6f, .4f, .2f, .1f };

				const float LastFrequencyIndex = static_cast<float>(ToneFrequencyMap.Num() - 1);

				const int32 ToneFrequencyPicker = static_cast<int32>(FMath::GetMappedRangeValueClamped({ 0, MaxDist }, { LastFrequencyIndex, 0 }, static_cast<float>(Distance)));

				GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::Red, FString::Printf(TEXT("Rarity: %d, Distance: %d"), Rarity, Distance));

				if (LastBeepTime > (ToneFrequencyMap[ToneFrequencyPicker] - KINDA_SMALL_NUMBER))
				{
					LastBeepTime = 0.f;
					UGameplayStatics::PlaySound2D(this, DetectionSound, ToneVolume, TonePitch);
				}
			}

			WGS->DebugDrawPosition(ProbePosition, .1f, FColor::Green);
		}
	}
}

void ATool_MetalDetector::OnActivate()
{
	if (ProbePositionCache.IsSet())
	{
		AGridMarker::NewMarker(this, MarkerClass, ProbePositionCache.GetValue(), Cast<APawn>(GetOwner()));
	}
}

DECLARE_LOG_CATEGORY_CLASS(LogGridMarker, Log, All)

AGridMarker* AGridMarker::NewMarker(const UObject* WorldContextObject, TSubclassOf<AGridMarker> MarkerClass, const FGridVector& SpawnPosition, APawn* Instigator)
{
	FWorldGridActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Instigator;
	SpawnParams.bCanAdjustPosition = false;
	SpawnParams.DesiredPosition = SpawnPosition;

	return Cast<AGridMarker>(UWorldGridSubsystem::Get(WorldContextObject)->TrySpawnSmallActorOnGrid(MarkerClass, SpawnParams));
}

AGridMarker::AGridMarker()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
}
