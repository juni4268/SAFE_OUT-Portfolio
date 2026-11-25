#include "CPRMissionWidget.h"
#include "CPREvent_AirCon.h"
#include "CPREvent_ExplosivesZone.h"
#include "CPREvent_FireElec.h"
#include "CPREvent_FireLithium.h"
#include "CPREvent_Overheat.h"
#include "CPREvent_Trap.h"
#include "CPREvent_Valve.h"
#include "CPREventSpawner.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCPRMissionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CollectEvents();
    RefreshAllStatuses();

    GetWorld()->GetTimerManager().SetTimer(CountRefreshTimerHandle, this, &UCPRMissionWidget::RefreshAllStatuses, 0.1f, true);
}

void UCPRMissionWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CountRefreshTimerHandle);
    }

    for (ACPREventBase* Event : AllTrackedEvents)
    {
        if (IsValid(Event))
        {
            Event->OnEventStatusChanged.RemoveDynamic(this, &UCPRMissionWidget::OnAnyEventStatusChanged);
        }
    }

    for (ACPREventSpawner* Spawner : Spawners)
    {
        if (IsValid(Spawner))
        {
            Spawner->OnEventSpawned.RemoveDynamic(this, &UCPRMissionWidget::OnSpawnerSpawnedEvent);
        }
    }

    Super::NativeDestruct();
}

void UCPRMissionWidget::CollectEvents()
{
    TArray<AActor *> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPREventBase::StaticClass(), FoundActors);

    for (AActor *Actor : FoundActors)
    {
        ACPREventBase *Event = Cast<ACPREventBase>(Actor);
        if (Event && !AllTrackedEvents.Contains(Event))
        {
            AllTrackedEvents.Add(Event);
            Event->OnEventStatusChanged.AddDynamic(this, &UCPRMissionWidget::OnAnyEventStatusChanged);

            if (ACPREvent_Valve *Valve = Cast<ACPREvent_Valve>(Event))
            {
                ValveEvents.Add(Valve);
            }
            else if (ACPREvent_FireElec *FireElec = Cast<ACPREvent_FireElec>(Event))
            {
                FireElecEvents.Add(FireElec);
            }
            else if (ACPREvent_AirCon *AirCon = Cast<ACPREvent_AirCon>(Event))
            {
                AirConEvents.Add(AirCon);
            }
            else if (ACPREvent_Overheat *Overheat = Cast<ACPREvent_Overheat>(Event))
            {
                OverheatEvents.Add(Overheat);
            }
            else if (ACPREvent_FireLithium *Lithium = Cast<ACPREvent_FireLithium>(Event))
            {
                LithiumEvents.Add(Lithium);
            }
            else if (ACPREvent_Trap *Trap = Cast<ACPREvent_Trap>(Event))
            {
                TrapEvents.Add(Trap);
            }
            else if (ACPREvent_ExplosivesZone *Explosives = Cast<ACPREvent_ExplosivesZone>(Event))
            {
                ExplosivesEvents.Add(Explosives);
            }
        }
    }

    TArray<AActor *> FoundSpawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPREventSpawner::StaticClass(), FoundSpawners);
    for (AActor *Actor : FoundSpawners)
    {
        ACPREventSpawner *Spawner = Cast<ACPREventSpawner>(Actor);
        if (Spawner && !Spawners.Contains(Spawner))
        {
            Spawners.Add(Spawner);

            Spawner->OnEventSpawned.AddUniqueDynamic(this, &UCPRMissionWidget::OnSpawnerSpawnedEvent);
        }
    }
}

void UCPRMissionWidget::OnAnyEventStatusChanged(ECPREventStatus NewStatus)
{
    if (NewStatus == ECPREventStatus::Resolved && MissionCompleteSound)
    {
        UGameplayStatics::PlaySound2D(this, MissionCompleteSound);
    }

    RefreshAllStatuses();
}

void UCPRMissionWidget::OnSpawnerSpawnedEvent(ACPREventBase* SpawnedEvent)
{
    if (!SpawnedEvent) return;

    if (!AllTrackedEvents.Contains(SpawnedEvent))
    {
        AllTrackedEvents.Add(SpawnedEvent);
        SpawnedEvent->OnEventStatusChanged.AddDynamic(this, &UCPRMissionWidget::OnAnyEventStatusChanged);

        if (ACPREvent_Valve *Valve = Cast<ACPREvent_Valve>(SpawnedEvent))
            ValveEvents.Add(Valve);
        else if (ACPREvent_FireElec *FireElec = Cast<ACPREvent_FireElec>(SpawnedEvent))
            FireElecEvents.Add(FireElec);
        else if (ACPREvent_AirCon *AirCon = Cast<ACPREvent_AirCon>(SpawnedEvent))
            AirConEvents.Add(AirCon);
        else if (ACPREvent_Overheat *Overheat = Cast<ACPREvent_Overheat>(SpawnedEvent))
            OverheatEvents.Add(Overheat);
        else if (ACPREvent_FireLithium *Lithium = Cast<ACPREvent_FireLithium>(SpawnedEvent))
            LithiumEvents.Add(Lithium);
        else if (ACPREvent_Trap *Trap = Cast<ACPREvent_Trap>(SpawnedEvent))
            TrapEvents.Add(Trap);
        else if (ACPREvent_ExplosivesZone *Explosives = Cast<ACPREvent_ExplosivesZone>(SpawnedEvent))
            ExplosivesEvents.Add(Explosives);

        RefreshAllStatuses();
    }
}

void UCPRMissionWidget::RefreshAllStatuses()
{
    // Valve
    SetTypeImages(Img_Valve_Resolved, Img_Valve_Unresolved, AreAllResolved(ValveEvents));

    // FireElec
    SetTypeImages(Img_FireElec_Resolved, Img_FireElec_Unresolved, AreAllResolved(FireElecEvents));

    // AirCon
    SetTypeImages(Img_AirCon_Resolved, Img_AirCon_Unresolved, AreAllResolved(AirConEvents));

    SetTypeImages(Img_Overheat_Resolved, Img_Overheat_Unresolved, AreAllOverheatsStable());

    bool bLithiumResolved = AreAllResolved(LithiumEvents);
    SetTypeImages(Img_Lithium_Resolved, Img_Lithium_Unresolved, bLithiumResolved);

    if (Text_Lithium)
    {
        int32 Current = 0;
        int32 Max = 0;
        for (ACPREvent_FireLithium *Lithium : LithiumEvents)
        {
            if (!IsValid(Lithium))
            {
                continue;
            }

            Current += FMath::FloorToInt(Lithium->LithiumCurrent);
            Max += FMath::FloorToInt(Lithium->LithiumMax);
        }
        Text_Lithium->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max)));
    }

    int32 MaxTrapCount = 0;
    for (ACPREventSpawner *Spawner : Spawners)
    {
        if (IsValid(Spawner))
        {
            MaxTrapCount += Spawner->GetPlannedSpawnCount(ACPREvent_Trap::StaticClass());
        }
    }
    MaxTrapCount = FMath::Max(MaxTrapCount, TrapEvents.Num());

    for (ACPREvent_Trap *Trap : TrapEvents)
    {
        if (IsValid(Trap) && Trap->IsResolved() && !CountedResolvedTrapEvents.Contains(Trap))
        {
            CountedResolvedTrapEvents.Add(Trap);
            ++ResolvedTrapCount;
        }
    }

    bool bTrapResolved = false;
    if (MaxTrapCount > 0 && ResolvedTrapCount >= MaxTrapCount)
    {
        bTrapResolved = true;
    }
    SetTypeImages(Img_Trap_Resolved, Img_Trap_Unresolved, bTrapResolved);

    if (Text_Trap)
    {

        Text_Trap->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), ResolvedTrapCount, MaxTrapCount)));
    }

    bool bExplosivesResolved = AreAllResolved(ExplosivesEvents);
    SetTypeImages(Img_Explosives_Resolved, Img_Explosives_Unresolved, bExplosivesResolved);

    if (Text_Explosives)
    {
        int32 Current = 0;
        int32 Max = 0;
        for (ACPREvent_ExplosivesZone *Explosives : ExplosivesEvents)
        {
            if (!IsValid(Explosives))
            {
                continue;
            }

            Current += Explosives->CurrentCount;
            Max += Explosives->RequiredCount;
        }

        if (ExplosivesEvents.Num() > 0)
        {
            Text_Explosives->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max)));
        }
    }

    if (!bAllMissionsCompleted)
    {

        const bool bHasAnyEvent =
            ValveEvents.Num() > 0 ||
            FireElecEvents.Num() > 0 ||
            AirConEvents.Num() > 0 ||
            OverheatEvents.Num() > 0 ||
            LithiumEvents.Num() > 0 ||
            MaxTrapCount > 0 ||
            ExplosivesEvents.Num() > 0;

        bool bAllDone = true;
        if (ValveEvents.Num() > 0     && !AreAllResolved(ValveEvents))     { bAllDone = false; }
        if (FireElecEvents.Num() > 0  && !AreAllResolved(FireElecEvents))  { bAllDone = false; }
        if (AirConEvents.Num() > 0    && !AreAllResolved(AirConEvents))    { bAllDone = false; }
        if (OverheatEvents.Num() > 0  && !AreAllOverheatsStable())         { bAllDone = false; }
        if (LithiumEvents.Num() > 0   && !AreAllResolved(LithiumEvents))   { bAllDone = false; }
        if (MaxTrapCount > 0          && !bTrapResolved)                   { bAllDone = false; }
        if (ExplosivesEvents.Num() > 0 && !bExplosivesResolved)            { bAllDone = false; }

        if (bHasAnyEvent && bAllDone)
        {
            bAllMissionsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("[CPRMissionWidget] 모든 미션 완료 → OnAllMissionsCompleted 방송"));
            OnAllMissionsCompleted.Broadcast();
        }
    }
}

void UCPRMissionWidget::SetTypeImages(UImage *ResolvedImg, UImage *UnresolvedImg, bool bResolved)
{
    if (ResolvedImg)
    {
        ResolvedImg->SetVisibility(bResolved ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (UnresolvedImg)
    {
        UnresolvedImg->SetVisibility(bResolved ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
}

bool UCPRMissionWidget::AreAllOverheatsStable() const
{
    if (OverheatEvents.Num() == 0)
    {
        return false;
    }

    for (ACPREvent_Overheat *Overheat : OverheatEvents)
    {

        if (!IsValid(Overheat) || Overheat->bIsOnFire)
        {
            return false;
        }
    }
    return true;
}
