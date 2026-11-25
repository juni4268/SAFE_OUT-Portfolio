#include "CPREvent_AirCon.h"
#include "CPRCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"

ACPREvent_AirCon::ACPREvent_AirCon()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    bHasTimeLimit = false;
    bUseAutoStart = true;
    RequiredItemTag = FName("AirFilter");
}

void ACPREvent_AirCon::BeginPlay()
{
    Super::BeginPlay();

    if (TargetFogActor)
    {
        if (UExponentialHeightFogComponent *FogComp =
                TargetFogActor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogComp->SetFogDensity(0.0f);
        }
    }
}

void ACPREvent_AirCon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetFogActor)
    {
        return;
    }

    if (IsInProgress() && !bIsClearing)
    {
        CurrentDensity += DensityIncreaseRate * DeltaTime;
        UpdateFogVisuals();
        ReceiveFogUpdated(FMath::Clamp(CurrentDensity / MaxDensityThreshold, 0.f, 1.f));

        if (CurrentDensity >= MaxDensityThreshold)
        {
            OnTimerExpired();
        }
    }
    else if (bIsClearing)
    {
        CurrentDensity = FMath::FInterpTo(CurrentDensity, 0.0f, DeltaTime, ClearSpeed);
        UpdateFogVisuals();
        ReceiveFogUpdated(0.0f);

        if (CurrentDensity <= 0.001f)
        {
            CurrentDensity = 0.0f;
            UpdateFogVisuals();
            bIsClearing = false;
            ResolveEvent();
        }
    }
}

void ACPREvent_AirCon::ClearFog()
{
    if (IsInProgress())
    {
        bIsClearing = true;
    }
}

void ACPREvent_AirCon::UpdateFogVisuals()
{
    if (!TargetFogActor)
    {
        return;
    }

    if (UExponentialHeightFogComponent *FogComp =
            TargetFogActor->FindComponentByClass<UExponentialHeightFogComponent>())
    {
        FogComp->SetFogDensity(CurrentDensity);
    }
}
