#include "CPRCharacter.h"
#include "Blueprint/UserWidget.h"
#include "CPRActionComponent.h"
#include "CPRDieUI.h"
#include "CPREventBase.h"
#include "CPRExtinguishable.h"
#include "CPRHUDWidget.h"
#include "CPRHardHat.h"
#include "CPRPlayerController.h"
#include "CPRTodoZoneTrigger.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "DrawDebugHelpers.h"

ACPRCharacter::ACPRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    ActionComp = CreateDefaultSubobject<UCPRActionComponent>(TEXT("ActionComp"));
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    ChokingAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ChokingAudioComp"));
    ChokingAudioComp->SetupAttachment(RootComponent);
    ChokingAudioComp->bAutoActivate = false;
}

void ACPRCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocallyControlled())
    {
        if (HUDWidgetClass)
        {
            HUDWidget = CreateWidget<UCPRHUDWidget>(GetWorld(), HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport();
            }
        }

        if (DieUIClass)
        {
            DieUI = CreateWidget<UCPRDieUI>(GetWorld(), DieUIClass);
            if (DieUI)
            {
                DieUI->AddToViewport();
                DieUI->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
    if (ChokingAudioComp && ChokingSound)
    {
        ChokingAudioComp->SetSound(ChokingSound);
    }
}

void ACPRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (ActionComp)
    {
        ActionComp->CheckForItems();
    }

    if (bIsExtinguishing)
    {
        UpdateExtinguisher(DeltaTime);
    }

    if (bIsDead && SpringArm)
    {
        FVector PelvisLoc = GetMesh()->GetSocketLocation(TEXT("Pelvis"));
        FVector CameraLoc = SpringArm->GetComponentLocation();
        SpringArm->SetWorldLocation(FVector(PelvisLoc.X, PelvisLoc.Y, CameraLoc.Z));
    }
}

void ACPRCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (ActionComp)
    {
        if (UEnhancedInputComponent *EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
        {

            if (APlayerController *PC = Cast<APlayerController>(GetController()))
            {
                if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
                        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
                {
                    if (ActionComp->DefaultMappingContext)
                    {
                        Subsystem->AddMappingContext(ActionComp->DefaultMappingContext, 0);
                    }
                }
            }

            EIC->BindAction(ActionComp->MoveAction, ETriggerEvent::Triggered, this, &ACPRCharacter::InputActionMove);
            EIC->BindAction(ActionComp->LookAction, ETriggerEvent::Triggered, this, &ACPRCharacter::InputActionLook);
            EIC->BindAction(ActionComp->JumpAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionJump);
            EIC->BindAction(ActionComp->JumpAction, ETriggerEvent::Completed, this, &ACPRCharacter::InputActionJump);
            EIC->BindAction(ActionComp->SprintAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionSprint);
            EIC->BindAction(ActionComp->SprintAction, ETriggerEvent::Completed, this,
                            &ACPRCharacter::InputActionSprint);
            EIC->BindAction(ActionComp->PickUpAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionPickUp);
            EIC->BindAction(ActionComp->UseAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionUse);
            EIC->BindAction(ActionComp->UseAction, ETriggerEvent::Completed, this,
                            &ACPRCharacter::InputActionUseReleased);
            EIC->BindAction(ActionComp->DropAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionDrop);
            EIC->BindAction(ActionComp->PauseAction, ETriggerEvent::Started, this, &ACPRCharacter::InputActionPause);
        }
    }
}

void ACPRCharacter::Landed(const FHitResult &Hit)
{
    Super::Landed(Hit);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    }
}

void ACPRCharacter::InputActionPickUp(const FInputActionValue &Value)
{
    if (ActionComp)
    {
        ActionComp->ActionPickUp();
    }
}

void ACPRCharacter::InputActionDrop()
{
    if (ActionComp)
    {
        ActionComp->ActionDrop();
    }
}

void ACPRCharacter::InputActionMove(const FInputActionValue &Value)
{
    if (ActionComp)
    {
        ActionComp->ActionPlayerMove(Value.Get<FVector2D>());
    }
}

void ACPRCharacter::InputActionLook(const FInputActionValue &Value)
{
    if (ActionComp)
    {
        ActionComp->ActionLookAndTurn(Value.Get<FVector2D>());
    }
}

void ACPRCharacter::InputActionJump(const FInputActionValue &Value)
{
    if (ActionComp)
    {
        ActionComp->ActionJump(Value.Get<bool>());
    }
}

void ACPRCharacter::InputActionSprint(const FInputActionValue &Value)
{
    if (ActionComp)
    {
        ActionComp->ActionSprint(Value.Get<bool>());
    }
}

void ACPRCharacter::InputActionPause(const FInputActionValue &Value)
{
    if (HUDWidget)
    {
        HUDWidget->TogglePauseMenu();
    }
}

void ACPRCharacter::InputActionUse(const FInputActionValue &Value)
{

    if (ActiveTodoZone)
    {
        ActiveTodoZone->OpenTodoZone();
        return;
    }

    if (CurrentHeldItem && CurrentHeldItem->ItemTag.ToString().StartsWith(TEXT("Extinguisher")))
    {
        StartExtinguisher();
        return;
    }

    if (ActionComp)
    {
        ActionComp->ActionUse();
    }
}

void ACPRCharacter::InputActionUseReleased(const FInputActionValue &Value)
{
    if (bIsExtinguishing)
    {
        StopExtinguisher();
    }
    else if (ActiveEvent)
    {
        ActiveEvent->ReceivePlayerInteractionReleased(this);
    }
}

void ACPRCharacter::UpdateInteractUI()
{
    if (HUDWidget)
    {

        bool bCanInteract = (ActiveEvent != nullptr) || (ActiveTodoZone != nullptr);
        HUDWidget->UpdateInteractVisibility(bCanInteract);
    }
}

void ACPRCharacter::ShowErrorMessage(FText Message, float DisplayTime)
{
    if (HUDWidget)
    {
        HUDWidget->ShowErrorMessage(Message, DisplayTime);
    }
}

void ACPRCharacter::EquipHardHat(UItemData *ItemData)
{
    if (!ItemData)
    {
        return;
    }

    if (AttachedHardHat)
    {
        AttachedHardHat->Destroy();
        AttachedHardHat = nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AttachedHardHat = GetWorld()->SpawnActor<ACPRHardHat>(ACPRHardHat::StaticClass(), GetActorLocation(),
                                                          GetActorRotation(), SpawnParams);
    if (AttachedHardHat)
    {
        AttachedHardHat->InitializeHardHat(ItemData);
        const FName HeadSocketName = TEXT("headSocket");
        AttachedHardHat->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                           HeadSocketName);
        bIsWearingHardHat = true;
    }
}

void ACPRCharacter::EnableRagdoll()
{
    OnChokingSound(false);
    StopExtinguisher();

    if (UCharacterMovementComponent *MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
        MoveComp->SetComponentTickEnabled(false);
    }

    if (UCapsuleComponent *Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    if (USkeletalMeshComponent *CharacterMesh = GetMesh())
    {
        CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
        CharacterMesh->SetSimulatePhysics(true);
        CharacterMesh->SetAllBodiesBelowSimulatePhysics(TEXT("Pelvis"), true, true);
        CharacterMesh->bBlendPhysics = true;
        CharacterMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    if (APlayerController *PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }

    bIsDead = true;
    SetupDeathCamera();
}

void ACPRCharacter::SetupDeathCamera()
{
    if (SpringArm)
    {

        SpringArm->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

        SpringArm->bUsePawnControlRotation = false;

        SpringArm->SetAbsolute(true, true, false);

        FRotator CurrentRotation = SpringArm->GetComponentRotation();
        SpringArm->SetWorldRotation(FRotator(-80.0f, CurrentRotation.Yaw, 0.0f));

        SpringArm->TargetOffset = FVector(0, 0, 50.0f);
        SpringArm->TargetArmLength = 250.0f;
        SpringArm->bDoCollisionTest = true;

        SpringArm->bEnableCameraLag = false;
    }
}

void ACPRCharacter::SetCharacterVisibility(bool bVisible, bool bShowUI)
{
    SetActorHiddenInGame(!bVisible);
    if (AttachedHardHat)
    {
        AttachedHardHat->SetActorHiddenInGame(!bVisible);
    }
    if (HUDWidget)
    {
        HUDWidget->SetVisibility(bShowUI ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void ACPRCharacter::StartExtinguisher()
{
    if (!bIsExtinguishing)
    {
        bIsExtinguishing = true;
        OnExtinguisherStart();
    }
}

void ACPRCharacter::StopExtinguisher()
{
    if (bIsExtinguishing)
    {
        bIsExtinguishing = false;
        OnExtinguisherStop();
    }
}

void ACPRCharacter::UpdateExtinguisher(float DeltaTime)
{
    const FVector StartLoc = GetActorLocation() + FVector(0.f, 0.f, 50.f);
    const FVector ForwardVector = GetActorForwardVector();
    const FVector TraceEnd = StartLoc + ForwardVector * ExtinguisherRange;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExtinguisherRadius);

    if (GetWorld()->SweepSingleByChannel(HitResult, StartLoc, TraceEnd, FQuat::Identity, ECC_Visibility, SphereShape,
                                         Params))
    {
        AActor *HitActor = HitResult.GetActor();
        if (!HitActor)
        {
            return;
        }

        ICPRExtinguishable *Extinguishable = Cast<ICPRExtinguishable>(HitActor);
        if (!Extinguishable)
        {
            Extinguishable = Cast<ICPRExtinguishable>(HitActor->GetOwner());
        }

        if (Extinguishable && CurrentHeldItem)
        {
            const FName HeldTag = CurrentHeldItem->ItemTag;
            ICPRExtinguishable::Execute_ApplyExtinguisher(Extinguishable->_getUObject(), ExtinguisherPower * DeltaTime,
                                                          HeldTag);
        }
    }
}

void ACPRCharacter::OnChokingSound(bool bIsChoking)
{
    if (ChokingAudioComp)
    {
        if (bIsChoking)
        {
            if (!ChokingAudioComp->IsPlaying())
            {
                ChokingAudioComp->Play();
            }
        }
        else
        {
            ChokingAudioComp->Stop();
        }
    }
}