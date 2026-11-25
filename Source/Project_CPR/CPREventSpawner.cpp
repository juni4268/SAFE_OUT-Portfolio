#include "CPREventSpawner.h"
#include "CPREventBase.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACPREventSpawner::ACPREventSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

int32 ACPREventSpawner::GetPlannedSpawnCount(TSubclassOf<ACPREventBase> TargetClass) const
{
    if (!TargetClass)
    {
        return 0;
    }

    int32 TotalCount = 0;
    for (const FEventSpawnInfo& Info : EventRoster)
    {
        if (Info.EventClass && Info.EventClass->IsChildOf(TargetClass))
        {
            TotalCount += Info.SpawnCount;
        }
    }
    return TotalCount;
}

void ACPREventSpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	UsedLocations.Empty();

	for (const FEventSpawnInfo& Info : EventRoster)
	{

		if (!Info.EventClass || Info.SpawnPointTag.IsNone() || Info.SpawnCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPREventSpawner] EventRoster에 EventClass 또는 SpawnPointTag가 비어있는 행이 있습니다. 건너뜁니다."));
			continue;
		}

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(World, Info.SpawnPointTag, FoundActors);

		if (FoundActors.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPREventSpawner] 태그 '%s'를 가진 스폰 포인트 액터가 맵에 없습니다."), *Info.SpawnPointTag.ToString());
			continue;
		}

		TArray<AActor*> AvailableActors;
		for (AActor* Candidate : FoundActors)
		{
			if (Candidate && !IsLocationUsed(Candidate->GetActorLocation()))
			{
				AvailableActors.Add(Candidate);
			}
		}

		if (AvailableActors.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPREventSpawner] 태그 '%s'의 스폰 포인트가 모두 사용 중이거나 없습니다. 이 이벤트를 스킵합니다."), *Info.SpawnPointTag.ToString());
			continue;
		}

		// Fisher-Yates shuffle
		for (int32 i = AvailableActors.Num() - 1; i > 0; --i)
		{
			int32 j = FMath::RandRange(0, i);
			AvailableActors.Swap(i, j);
		}

		const int32 ActualSpawnCount = FMath::Min(Info.SpawnCount, AvailableActors.Num());

		if (ActualSpawnCount < Info.SpawnCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPREventSpawner] 태그 '%s'에 요청한 스폰 개수(%d)보다 가용 위치(%d)가 부족합니다. %d개만 스폰합니다."),
				*Info.SpawnPointTag.ToString(), Info.SpawnCount, AvailableActors.Num(), ActualSpawnCount);
		}

		for (int32 i = 0; i < ActualSpawnCount; ++i)
		{
			AActor* SpawnPoint = AvailableActors[i];
			if (!SpawnPoint) continue;

			const FVector SpawnLoc = SpawnPoint->GetActorLocation();
			const FRotator SpawnRot = SpawnPoint->GetActorRotation();

			UsedLocations.Add(SpawnLoc);

			const float DelayMin = FMath::Max(0.0f, Info.SpawnDelayMin);
			const float DelayMax = FMath::Max(DelayMin, Info.SpawnDelayMax);
			const float Delay    = FMath::IsNearlyZero(DelayMax)
				? 0.0f
				: FMath::FRandRange(DelayMin, DelayMax);

			if (FMath::IsNearlyZero(Delay))
			{

				SpawnEventDeferred(Info.EventClass, SpawnLoc, SpawnRot);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[CPREventSpawner] '%s' 이벤트를 %.2f초 후에 %s에 스폰 예약합니다."),
					*Info.EventClass->GetName(), Delay, *SpawnLoc.ToString());

				FTimerHandle DummyHandle;
				FTimerDelegate SpawnDelegate;
				SpawnDelegate.BindUFunction(this, FName("SpawnEventDeferred"), Info.EventClass, SpawnLoc, SpawnRot);
				GetWorldTimerManager().SetTimer(DummyHandle, SpawnDelegate, Delay, false);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[CPREventSpawner] 총 %d개의 이벤트 스폰 완료."), SpawnedEvents.Num());
}

bool ACPREventSpawner::IsLocationUsed(const FVector& Location) const
{

	constexpr float ToleranceSq = 1.0f * 1.0f;

	for (const FVector& Used : UsedLocations)
	{
		if (FVector::DistSquared(Used, Location) <= ToleranceSq)
		{
			return true;
		}
	}
	return false;
}

void ACPREventSpawner::SpawnEventDeferred(TSubclassOf<ACPREventBase> EventClass, FVector SpawnLoc, FRotator SpawnRot)
{
	UWorld* World = GetWorld();
	if (!World || !EventClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACPREventBase* SpawnedEvent = World->SpawnActor<ACPREventBase>(EventClass, SpawnLoc, SpawnRot, SpawnParams);
	if (SpawnedEvent)
	{
		SpawnedEvents.Add(SpawnedEvent);
		OnEventSpawned.Broadcast(SpawnedEvent);

		UE_LOG(LogTemp, Log, TEXT("[CPREventSpawner] '%s' 이벤트를 위치 %s에 스폰했습니다."),
			*EventClass->GetName(), *SpawnLoc.ToString());
	}
}
