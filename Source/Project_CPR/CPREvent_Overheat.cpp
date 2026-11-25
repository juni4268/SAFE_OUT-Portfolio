#include "CPREvent_Overheat.h"
#include "Animation/AnimInstance.h"
#include "CPRCharacter.h"
#include "CPRHUDWidget.h"
#include "CPROverheatWidget.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACPREvent_Overheat::ACPREvent_Overheat()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    bUseAutoStart = true;
    AutoStartDelay = 3.0f;
    bHasTimeLimit = false;
    TimeLimit = 30.0f;

    BurnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BurnZone"));
    BurnZone->SetupAttachment(RootComponent);
    BurnZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
    BurnZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BurnZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    BurnZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    WarningAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("WarningAudioComp"));
    WarningAudioComp->SetupAttachment(RootComponent);
    WarningAudioComp->bAutoActivate = false;

    OilFillAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("OilFillAudioComp"));
    OilFillAudioComp->SetupAttachment(RootComponent);
    OilFillAudioComp->bAutoActivate = false;

    CoolantFillAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("CoolantFillAudioComp"));
    CoolantFillAudioComp->SetupAttachment(RootComponent);
    CoolantFillAudioComp->bAutoActivate = false;

    FireAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComp"));
    FireAudioComp->SetupAttachment(RootComponent);
    FireAudioComp->bAutoActivate = false;

    OverheatWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheatWidgetComp"));
    OverheatWidgetComp->SetupAttachment(RootComponent);
    OverheatWidgetComp->SetWidgetSpace(EWidgetSpace::World);
    OverheatWidgetComp->SetDrawSize(FVector2D(300.f, 150.f));
    OverheatWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OverheatWidgetComp_Back = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheatWidgetComp_Back"));
    OverheatWidgetComp_Back->SetupAttachment(RootComponent);
    OverheatWidgetComp_Back->SetWidgetSpace(EWidgetSpace::World);
    OverheatWidgetComp_Back->SetDrawSize(FVector2D(300.f, 150.f));
    OverheatWidgetComp_Back->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OverheatWidgetComp_Back->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
}

void ACPREvent_Overheat::BeginPlay()
{
    Super::BeginPlay();

    OilCurrent = FMath::FRandRange(OilStartRangeMin, OilStartRangeMax);
    CoolantCurrent = FMath::FRandRange(CoolantStartRangeMin, CoolantStartRangeMax);
    TempCurrent = TempMin;

    if (BurnZone)
    {
        BurnZone->OnComponentBeginOverlap.AddDynamic(this, &ACPREvent_Overheat::OnBurnZoneBeginOverlap);
    }

    if (WarningAudioComp && HighTempWarningSound)
    {
        WarningAudioComp->SetSound(HighTempWarningSound);
    }
    if (OilFillAudioComp && OilFillSound)
    {
        OilFillAudioComp->SetSound(OilFillSound);
    }
    if (CoolantFillAudioComp && CoolantFillSound)
    {
        CoolantFillAudioComp->SetSound(CoolantFillSound);
    }
    if (FireAudioComp && FireSound)
    {
        FireAudioComp->SetSound(FireSound);
    }

    if (OverheatWidgetComp)
    {
        if (UCPROverheatWidget* WorldWidget = Cast<UCPROverheatWidget>(OverheatWidgetComp->GetUserWidgetObject()))
        {
            WorldWidget->ShowPanel(true, this);
        }
    }

    if (OverheatWidgetComp_Back)
    {
        if (UCPROverheatWidget* WorldWidgetBack = Cast<UCPROverheatWidget>(OverheatWidgetComp_Back->GetUserWidgetObject()))
        {
            WorldWidgetBack->ShowPanel(true, this);
        }
    }
}

void ACPREvent_Overheat::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{

    ACPRCharacter *CharacterSnapshot = CachedCharacter;

    Super::OnStatusChanged(OldStatus, NewStatus);

    if (NewStatus == ECPREventStatus::Failed || NewStatus == ECPREventStatus::Resolved)
    {
        bIsOnFire = false;
        StopFillingOil();
        StopFillingCoolant();

        if (WarningAudioComp && WarningAudioComp->IsPlaying())
        {
            WarningAudioComp->Stop();
        }

        if (FireAudioComp && FireAudioComp->IsPlaying())
        {
            FireAudioComp->Stop();
        }

        if (BurnZone)
        {
            BurnZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        if (CharacterSnapshot && CharacterSnapshot->HUDWidget)
        {
            CharacterSnapshot->HUDWidget->ShowOverheatPanel(false);
        }

        if (OverheatWidgetComp)
        {
            if (UCPROverheatWidget* WorldWidget = Cast<UCPROverheatWidget>(OverheatWidgetComp->GetUserWidgetObject()))
            {
                WorldWidget->ShowPanel(false);
            }
        }

        if (OverheatWidgetComp_Back)
        {
            if (UCPROverheatWidget* WorldWidgetBack = Cast<UCPROverheatWidget>(OverheatWidgetComp_Back->GetUserWidgetObject()))
            {
                WorldWidgetBack->ShowPanel(false);
            }
        }

        EndOverheatInteraction();
    }
}

void ACPREvent_Overheat::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bSuspendedForClear)
    {
        return;
    }

    if (!IsInProgress())
    {
        return;
    }

    if (CurrentPhase == EOverheatPhase::Managing)
    {
        const bool bOilEmpty     = (OilCurrent <= 0.f);
        const bool bCoolantEmpty = (CoolantCurrent <= 0.f);

        if (bOilEmpty || bCoolantEmpty)
        {

            const float RiseRate = (bOilEmpty && bCoolantEmpty) ? TempRiseBothEmpty : TempRiseRateOneEmpty;
            TempCurrent += RiseRate * DeltaTime;
        }
        else
        {

            const float OilRatio     = (OilMax > 0.f)     ? (OilCurrent / OilMax)         : 0.f;
            const float CoolantRatio = (CoolantMax > 0.f) ? (CoolantCurrent / CoolantMax)  : 0.f;
            if (OilRatio >= TempDecreaseThresholdRatio && CoolantRatio >= TempDecreaseThresholdRatio)
            {
                TempCurrent -= TempDecreaseRate * DeltaTime;
            }
        }

        TempCurrent = FMath::Clamp(TempCurrent, TempMin, TempMax);

        if (!bIsFillingOil)
        {
            OilCurrent = FMath::Max(0.f, OilCurrent - OilConsumeRate * DeltaTime);
        }
        if (!bIsFillingCoolant)
        {
            CoolantCurrent = FMath::Max(0.f, CoolantCurrent - CoolantConsumeRate * DeltaTime);
        }

        if (bIsFillingOil)
        {
            DoFillOilTick(DeltaTime);
        }
        if (bIsFillingCoolant)
        {
            DoFillCoolantTick(DeltaTime);
        }

        if (TempCurrent >= TempMax)
        {
            TriggerOverheat();
            return;
        }

        const float TempRange = TempMax - TempMin;
        const float TempRatio = (TempRange > 0.f) ? (TempCurrent - TempMin) / TempRange : 0.f;
        if (TempRatio >= 0.7f)
        {
            if (WarningAudioComp && !WarningAudioComp->IsPlaying() && WarningAudioComp->Sound)
            {
                WarningAudioComp->Play();
            }
        }
        else
        {
            if (WarningAudioComp && WarningAudioComp->IsPlaying())
            {
                WarningAudioComp->Stop();
            }
        }
    }

    UpdateOverheatUI();

    if (!bIsOnFire && MarkerTexture_Overheat)
    {
        const bool bShouldShow = (TempCurrent > TempMin);

        if (bShouldShow != bOverheatMarkerShown)
        {
            bOverheatMarkerShown = bShouldShow;
            UpdateMarkerTexture();
        }
    }
}

void ACPREvent_Overheat::TriggerOverheat()
{
    if (bSuspendedForClear || bIsOnFire)
    {
        return;
    }

    bIsOnFire = true;
    CurrentPhase = EOverheatPhase::OnFire;
    FireLevel = 1.0f;

    EndOverheatInteraction();
    StopFillingOil();
    StopFillingCoolant();

    if (WarningAudioComp && WarningAudioComp->IsPlaying())
    {
        WarningAudioComp->Stop();
    }

    if (FireAudioComp && !FireAudioComp->IsPlaying() && FireAudioComp->Sound)
    {
        FireAudioComp->Play();
    }

    if (CachedCharacter && CachedCharacter->HUDWidget)
    {
        CachedCharacter->HUDWidget->ShowOverheatPanel(false);
        CachedCharacter->HUDWidget->UpdateInteractVisibility(true);
    }

    if (UWorld *World = GetWorld())
    {
        World->GetTimerManager().SetTimer(EventTimerHandle, this, &ACPREvent_Overheat::OnTimerExpired, TimeLimit,
                                          false);
    }

    ReceiveOverheatStarted();
    ReceiveFireLevelChanged(FireLevel);

    UpdateMarkerTexture();

    if (BurnZone)
    {
        BurnZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        TArray<AActor *> OverlappingActors;
        BurnZone->GetOverlappingActors(OverlappingActors, ACPRCharacter::StaticClass());

        if (OverlappingActors.Num() > 0)
        {
            ReceiveOverheatBurnedDeath();
            SetEventStatus(ECPREventStatus::Failed);
            HandleCustomDieSequence(BurnedDeathSettings);
        }
    }
}

void ACPREvent_Overheat::OnTimerExpired()
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (!bIsOnFire)
    {
        return;
    }
    if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
    {
        return;
    }

    ReceiveEventTimeOut();
    SetEventStatus(ECPREventStatus::Failed);
    HandleCustomDieSequence(TimeOutDeathSettings);
}

void ACPREvent_Overheat::SuspendForClear()
{
    if (bSuspendedForClear)
    {
        return;
    }

    bSuspendedForClear = true;

    if (CurrentStatus == ECPREventStatus::Interacting)
    {
        EndOverheatInteraction();
    }

    StopFillingOil();
    StopFillingCoolant();

    if (WarningAudioComp && WarningAudioComp->IsPlaying())
    {
        WarningAudioComp->Stop();
    }

    if (OilFillAudioComp && OilFillAudioComp->IsPlaying())
    {
        OilFillAudioComp->Stop();
    }

    if (CoolantFillAudioComp && CoolantFillAudioComp->IsPlaying())
    {
        CoolantFillAudioComp->Stop();
    }

    if (FireAudioComp && FireAudioComp->IsPlaying())
    {
        FireAudioComp->Stop();
    }

    if (BurnZone)
    {
        BurnZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EventTimerHandle);
        World->GetTimerManager().ClearTimer(AutoStartTimerHandle);
    }

    if (CachedCharacter)
    {
        if (CachedCharacter->ActiveEvent == this)
        {
            CachedCharacter->ActiveEvent = nullptr;
        }

        if (CachedCharacter->HUDWidget)
        {
            CachedCharacter->HUDWidget->ShowOverheatPanel(false);
            CachedCharacter->HUDWidget->ShowOverheatButtonBox(false);
            CachedCharacter->HUDWidget->UpdateInteractVisibility(false);
        }
    }

    SetActorTickEnabled(false);
}

void ACPREvent_Overheat::OnInteractionZoneBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                                       UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                                                       bool bFromSweep, const FHitResult &SweepResult)
{
    if (bSuspendedForClear)
    {
        return;
    }

    ACPRCharacter *Character = Cast<ACPRCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    bIsPlayerInRange = true;
    Character->ActiveEvent = this;
    CachedCharacter = Character;

    if (Character->HUDWidget)
    {

        if (CurrentPhase == EOverheatPhase::Managing)
        {
            Character->HUDWidget->UpdateInteractVisibility(true);
            UpdateOverheatUI();
        }
        else if (CurrentPhase == EOverheatPhase::OnFire)
        {
            Character->HUDWidget->UpdateInteractVisibility(true);
        }
    }
}

void ACPREvent_Overheat::OnInteractionZoneEndOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                                     UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
    ACPRCharacter *Character = Cast<ACPRCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    bIsPlayerInRange = false;
    EndOverheatInteraction();
    StopFillingOil();
    StopFillingCoolant();

    if (Character->ActiveEvent == this)
    {
        Character->ActiveEvent = nullptr;

        if (Character->HUDWidget)
        {

            Character->HUDWidget->ShowOverheatPanel(false);
            Character->HUDWidget->UpdateInteractVisibility(false);
        }
    }
}

void ACPREvent_Overheat::OnBurnZoneBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                                UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                const FHitResult &SweepResult)
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (!bIsOnFire || CurrentStatus == ECPREventStatus::Failed || CurrentStatus == ECPREventStatus::Resolved)
    {
        return;
    }
    if (ACPRCharacter *Character = Cast<ACPRCharacter>(OtherActor))
    {
        ReceiveOverheatBurnedDeath();
        SetEventStatus(ECPREventStatus::Failed);
        HandleCustomDieSequence(BurnedDeathSettings);
    }
}

void ACPREvent_Overheat::StartOverheatInteraction(ACPRCharacter *TargetCharacter)
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (CurrentPhase != EOverheatPhase::Managing || CurrentStatus != ECPREventStatus::Active || !TargetCharacter)
    {
        return;
    }

    CachedCharacter = TargetCharacter;
    TargetCharacter->ActiveEvent = this;

    APlayerController *PC = Cast<APlayerController>(CachedCharacter->Controller);
    if (!PC)
    {
        return;
    }

    SetEventStatus(ECPREventStatus::Interacting);
    CachedCharacter = TargetCharacter;

    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    PC->bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(InputMode);

    if (OverheatMappingContext)
    {
        if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(OverheatMappingContext, 1);
        }
    }

    EnableInput(PC);
    if (UEnhancedInputComponent *EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EIC->ClearActionBindings();
        if (FillOilAction)
        {
            EIC->BindAction(FillOilAction, ETriggerEvent::Started, this, &ACPREvent_Overheat::OnInputFillOilStarted);
            EIC->BindAction(FillOilAction, ETriggerEvent::Completed, this, &ACPREvent_Overheat::OnInputFillOilEnded);
            EIC->BindAction(FillOilAction, ETriggerEvent::Canceled, this, &ACPREvent_Overheat::OnInputFillOilEnded);
        }
        if (FillCoolantAction)
        {
            EIC->BindAction(FillCoolantAction, ETriggerEvent::Started, this, &ACPREvent_Overheat::OnInputFillCoolantStarted);
            EIC->BindAction(FillCoolantAction, ETriggerEvent::Completed, this, &ACPREvent_Overheat::OnInputFillCoolantEnded);
            EIC->BindAction(FillCoolantAction, ETriggerEvent::Canceled, this, &ACPREvent_Overheat::OnInputFillCoolantEnded);
        }
        if (QuitOverheatAction)
        {
            EIC->BindAction(QuitOverheatAction, ETriggerEvent::Started, this, &ACPREvent_Overheat::OnInputQuitOverheat);
        }
    }

    if (CachedCharacter->HUDWidget)
    {

        CachedCharacter->HUDWidget->ShowOverheatButtonBox(true, this);
        CachedCharacter->HUDWidget->UpdateInteractVisibility(false);
    }
}

void ACPREvent_Overheat::EndOverheatInteraction()
{
    if (!CachedCharacter)
    {
        return;
    }

    APlayerController *PC = Cast<APlayerController>(CachedCharacter->Controller);
    if (PC)
    {

        if (OverheatMappingContext)
        {
            if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                Subsystem->RemoveMappingContext(OverheatMappingContext);
            }
        }
        DisableInput(PC);

        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    if (CurrentStatus != ECPREventStatus::Resolved && CurrentStatus != ECPREventStatus::Failed)
    {
        SetEventStatus(ECPREventStatus::Active);
    }

    if (CachedCharacter && CachedCharacter->HUDWidget)
    {

        CachedCharacter->HUDWidget->ShowOverheatButtonBox(false);
        if (bIsPlayerInRange)
        {
            CachedCharacter->HUDWidget->UpdateInteractVisibility(true);
        }
    }
}

void ACPREvent_Overheat::ReceivePlayerInteraction_Implementation(ACPRCharacter *TargetCharacter)
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (!TargetCharacter)
    {
        return;
    }

    if (CurrentPhase == EOverheatPhase::Managing && CurrentStatus == ECPREventStatus::Active)
    {
        StartOverheatInteraction(TargetCharacter);
        return;
    }

    if (CurrentPhase == EOverheatPhase::OnFire)
    {
        TargetCharacter->ShowErrorMessage(FText::FromString(TEXT("방화 장비(소화기)로 불을 꺼야 합니다!")));
    }
}

void ACPREvent_Overheat::ReceivePlayerInteractionReleased_Implementation(ACPRCharacter *TargetCharacter)
{
}

void ACPREvent_Overheat::StartFillingOil(ACPRCharacter *Character)
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (!Character || !IsInProgress() || CurrentPhase != EOverheatPhase::Managing)
    {
        return;
    }

    if (bIsFillingOil)
    {
        return;
    }

    UItemData *HeldItem = Character->CurrentHeldItem;
    if (!HeldItem)
    {
        Character->ShowErrorMessage(FText::FromString(TEXT("아이템이 없습니다!")));
        return;
    }
    if (HeldItem->ItemTag != OilItemTag)
    {
        Character->ShowErrorMessage(FText::FromString(TEXT("잘못된 아이템입니다!")));
        return;
    }

    bIsFillingOil = true;
    CachedCharacter = Character;

    if (OilFillAudioComp && !OilFillAudioComp->IsPlaying() && OilFillAudioComp->Sound)
    {
        OilFillAudioComp->Play();
    }

    if (Character->HUDWidget)
    {
        Character->HUDWidget->SimulateOilButtonPress(true);
    }

    if (OilFillAnimation && Character->GetMesh())
    {
        if (UAnimInstance *AnimInst = Character->GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Play(OilFillAnimation);
        }
    }
}

void ACPREvent_Overheat::StopFillingOil()
{
    if (!bIsFillingOil)
    {
        return;
    }
    bIsFillingOil = false;

    if (OilFillAudioComp && OilFillAudioComp->IsPlaying())
    {
        OilFillAudioComp->Stop();
    }

    if (CachedCharacter && CachedCharacter->HUDWidget)
    {
        CachedCharacter->HUDWidget->SimulateOilButtonPress(false);
    }

    if (OilFillAnimation && CachedCharacter && CachedCharacter->GetMesh())
    {
        if (UAnimInstance *AnimInst = CachedCharacter->GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Stop(0.15f, OilFillAnimation);
        }
    }
}

void ACPREvent_Overheat::StartFillingCoolant(ACPRCharacter *Character)
{
    if (bSuspendedForClear)
    {
        return;
    }

    if (!Character || !IsInProgress() || CurrentPhase != EOverheatPhase::Managing)
    {
        return;
    }

    if (bIsFillingCoolant)
    {
        return;
    }

    UItemData *HeldItem = Character->CurrentHeldItem;
    if (!HeldItem)
    {
        Character->ShowErrorMessage(FText::FromString(TEXT("아이템이 없습니다!")));
        return;
    }
    if (HeldItem->ItemTag != CoolantItemTag)
    {
        Character->ShowErrorMessage(FText::FromString(TEXT("잘못된 아이템입니다!")));
        return;
    }

    bIsFillingCoolant = true;
    CachedCharacter = Character;

    if (CoolantFillAudioComp && !CoolantFillAudioComp->IsPlaying() && CoolantFillAudioComp->Sound)
    {
        CoolantFillAudioComp->Play();
    }

    if (Character->HUDWidget)
    {
        Character->HUDWidget->SimulateCoolantButtonPress(true);
    }

    if (CoolantFillAnimation && Character->GetMesh())
    {
        if (UAnimInstance *AnimInst = Character->GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Play(CoolantFillAnimation);
        }
    }
}

void ACPREvent_Overheat::StopFillingCoolant()
{
    if (!bIsFillingCoolant)
    {
        return;
    }
    bIsFillingCoolant = false;

    if (CoolantFillAudioComp && CoolantFillAudioComp->IsPlaying())
    {
        CoolantFillAudioComp->Stop();
    }

    if (CachedCharacter && CachedCharacter->HUDWidget)
    {
        CachedCharacter->HUDWidget->SimulateCoolantButtonPress(false);
    }

    if (CoolantFillAnimation && CachedCharacter && CachedCharacter->GetMesh())
    {
        if (UAnimInstance *AnimInst = CachedCharacter->GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Stop(0.15f, CoolantFillAnimation);
        }
    }
}

void ACPREvent_Overheat::DoFillOilTick(float DeltaTime)
{
    OilCurrent = FMath::Min(OilCurrent + OilFillRate * DeltaTime, OilMax);

    if (OilFillAnimation && CachedCharacter && CachedCharacter->GetMesh())
    {
        if (UAnimInstance *AnimInst = CachedCharacter->GetMesh()->GetAnimInstance())
        {
            if (!AnimInst->Montage_IsPlaying(OilFillAnimation))
            {
                AnimInst->Montage_Play(OilFillAnimation);
            }
        }
    }
}

void ACPREvent_Overheat::DoFillCoolantTick(float DeltaTime)
{
    CoolantCurrent = FMath::Min(CoolantCurrent + CoolantFillRate * DeltaTime, CoolantMax);

    if (CoolantFillAnimation && CachedCharacter && CachedCharacter->GetMesh())
    {
        if (UAnimInstance *AnimInst = CachedCharacter->GetMesh()->GetAnimInstance())
        {
            if (!AnimInst->Montage_IsPlaying(CoolantFillAnimation))
            {
                AnimInst->Montage_Play(CoolantFillAnimation);
            }
        }
    }
}

bool ACPREvent_Overheat::ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag)
{
	if (ExtinguisherTag != FName("Extinguisher"))
	{
		return false;
	}
	DecreaseFireLevel(Amount);
	return true;
}

void ACPREvent_Overheat::DecreaseFireLevel(float Amount)
{
    if (!bIsOnFire || CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
    {
        return;
    }

    FireLevel -= Amount;
    FireLevel = FMath::Max(FireLevel, 0.f);
    ReceiveFireLevelChanged(FireLevel);

    if (FireLevel <= 0.f)
    {
        if (FireAudioComp && FireAudioComp->IsPlaying())
        {
            FireAudioComp->Stop();
        }
        ResolveEvent();
    }
}

void ACPREvent_Overheat::UpdateOverheatUI()
{
    if (CurrentPhase != EOverheatPhase::Managing)
    {
        return;
    }

    const float OilRatio     = (OilMax > 0.f)     ? (OilCurrent / OilMax)         : 0.f;
    const float CoolantRatio = (CoolantMax > 0.f) ? (CoolantCurrent / CoolantMax)  : 0.f;
    const float TempRange    = TempMax - TempMin;
    const float TempRatio    = (TempRange > 0.f)  ? (TempCurrent - TempMin) / TempRange : 0.f;

    if (OverheatWidgetComp)
    {
        if (UCPROverheatWidget* WorldWidget = Cast<UCPROverheatWidget>(OverheatWidgetComp->GetUserWidgetObject()))
        {

            if (WorldWidget->OwningEvent != this)
            {
                WorldWidget->ShowPanel(true, this);
                WorldWidget->ShowButtonBox(false);
            }
            WorldWidget->UpdateStats(OilRatio, CoolantRatio, TempRatio);
        }
    }

    if (OverheatWidgetComp_Back)
    {
        if (UCPROverheatWidget* WorldWidgetBack = Cast<UCPROverheatWidget>(OverheatWidgetComp_Back->GetUserWidgetObject()))
        {
            if (WorldWidgetBack->OwningEvent != this)
            {
                WorldWidgetBack->ShowPanel(true, this);
                WorldWidgetBack->ShowButtonBox(false);
            }
            WorldWidgetBack->UpdateStats(OilRatio, CoolantRatio, TempRatio);
        }
    }

}

void ACPREvent_Overheat::OnInputFillOilStarted(const FInputActionValue &Value)
{

    if (bIsFillingCoolant)
    {
        return;
    }
    StartFillingOil(CachedCharacter);
}

void ACPREvent_Overheat::OnInputFillOilEnded(const FInputActionValue &Value)
{
    StopFillingOil();
}

void ACPREvent_Overheat::OnInputFillCoolantStarted(const FInputActionValue &Value)
{

    if (bIsFillingOil)
    {
        return;
    }
    StartFillingCoolant(CachedCharacter);
}

void ACPREvent_Overheat::OnInputFillCoolantEnded(const FInputActionValue &Value)
{
    StopFillingCoolant();
}

void ACPREvent_Overheat::OnInputQuitOverheat(const FInputActionValue &Value)
{
    EndOverheatInteraction();
}

bool ACPREvent_Overheat::IsBlockingLevelClear() const
{

    return !bSuspendedForClear && bIsOnFire;
}

UTexture2D* ACPREvent_Overheat::GetOverrideMarkerTexture() const
{

    if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
    {
        return nullptr;
    }

    if (bIsOnFire)
    {
        return MarkerTexture_OnFire;
    }

    if (MarkerTexture_Overheat && TempCurrent > TempMin)
    {
        return MarkerTexture_Overheat;
    }

    return nullptr;
}

FMarkerAnimSettings* ACPREvent_Overheat::GetOverrideMarkerAnim()
{

    if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
    {
        return nullptr;
    }

    if (bIsOnFire)
    {
        return &Anim_OnFire;
    }
    else if (MarkerTexture_Overheat && TempCurrent > TempMin)
    {
        return &Anim_Overheat;
    }

    return nullptr;
}
