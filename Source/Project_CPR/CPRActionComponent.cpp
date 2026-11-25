#include "CPRActionComponent.h"
#include "CPRCharacter.h"
#include "CPREventBase.h"
#include "CPRHUDWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"

UCPRActionComponent::UCPRActionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HighlightMat(
        TEXT("/Game/Material/M_ItemHighlight.M_ItemHighlight"));
    if (HighlightMat.Succeeded())
    {
        ItemHighlightMaterial = HighlightMat.Object;
    }
}

void UCPRActionComponent::BeginPlay()
{
    Super::BeginPlay();
    Player = Cast<ACPRCharacter>(GetOwner());
}

void UCPRActionComponent::CheckForItems()
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (!Player || !Player->FollowCamera)
    {
        return;
    }

    FVector Start = Player->FollowCamera->GetComponentLocation();
    FVector End = Start + (Player->FollowCamera->GetForwardVector() * 600.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Player);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
    AItem *NewItem = bHit ? Cast<AItem>(HitResult.GetActor()) : nullptr;

    if (NewItem != CurrentTargetItem)
    {
        if (CurrentTargetItem)
        {
            CurrentTargetItem->SetHighlight(false);
        }

        if (NewItem && ItemHighlightMaterial)
        {
            NewItem->SetHighlight(true, ItemHighlightMaterial);
        }

        CurrentTargetItem = NewItem;

        if (Player)
        {
            Player->UpdateInteractUI();
        }
    }
}

void UCPRActionComponent::ActionPlayerMove(FVector2D MoveDirection2D)
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player && Player->GetController())
    {
        const FRotator Rotation = Player->GetController()->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        Player->AddMovementInput(ForwardDirection, MoveDirection2D.Y);
        Player->AddMovementInput(RightDirection, MoveDirection2D.X);
    }
}

void UCPRActionComponent::ActionLookAndTurn(FVector2D LookTurnDirection2D)
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player)
    {
        Player->AddControllerYawInput(LookTurnDirection2D.X);
        Player->AddControllerPitchInput(LookTurnDirection2D.Y);
    }
}

void UCPRActionComponent::ActionJump(bool IsJump)
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player)
    {
        if (IsJump)
        {
            Player->Jump();
        }
        else
        {
            Player->StopJumping();
        }
    }
}

void UCPRActionComponent::ActionSprint(bool IsSprint)
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player && Player->GetCharacterMovement())
    {
        Player->GetCharacterMovement()->MaxWalkSpeed = IsSprint ? SprintSpeed : WalkSpeed;
    }
}

void UCPRActionComponent::ActionPickUp()
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (!Player)
    {
        return;
    }

    if (CurrentTargetItem && CurrentTargetItem->ItemData)
    {
        if (CurrentTargetItem->ItemData->ItemTag == FName(TEXT("HardHat")))
        {
            Player->EquipHardHat(CurrentTargetItem->ItemData);

            if (PickupSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Player->GetActorLocation());
            }

            CurrentTargetItem->OnInteract();
            CurrentTargetItem = nullptr;
            Player->UpdateInteractUI();
            return;
        }

        if (Player->CurrentHeldItem && Player->ItemClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            const FVector SwapLoc = CurrentTargetItem->GetActorLocation();
            const FRotator SwapRot = CurrentTargetItem->GetActorRotation();

            AItem *DroppedItem = GetWorld()->SpawnActor<AItem>(Player->ItemClass, SwapLoc, SwapRot, SpawnParams);
            if (DroppedItem)
            {
                DroppedItem->InitializeItem(Player->CurrentHeldItem);
            }
        }

        Player->CurrentHeldItem = CurrentTargetItem->ItemData;

        if (Player->HUDWidget)
        {
            Player->HUDWidget->UpdateAcquisitionInfo(Player->CurrentHeldItem->Icon,
                                                     Player->CurrentHeldItem->DisplayName);
        }

        if (PickupSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Player->GetActorLocation());
        }

        CurrentTargetItem->OnInteract();
        CurrentTargetItem = nullptr;
        Player->UpdateInteractUI();
    }
}

void UCPRActionComponent::ActionUse()
{

    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player && Player->ActiveEvent)
    {
        Player->ActiveEvent->ReceivePlayerInteraction(Player);
    }
}

bool UCPRActionComponent::ActionDrop()
{
    if (!Player)
    {
        Player = Cast<ACPRCharacter>(GetOwner());
    }

    if (Player && Player->CurrentHeldItem && Player->ItemClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

        const FVector PlayerLoc = Player->GetActorLocation();
        const FVector Forward = Player->GetActorForwardVector();
        const FVector BaseLoc = PlayerLoc + (Forward * 150.0f);

        float WaistZ = PlayerLoc.Z;
        if (UCapsuleComponent *Capsule = Player->GetCapsuleComponent())
        {
            const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
            constexpr float WaistFromBottomFraction = 0.55f;
            WaistZ = PlayerLoc.Z - HalfHeight + (HalfHeight * WaistFromBottomFraction);
        }

        const FVector WaistStart(PlayerLoc.X, PlayerLoc.Y, WaistZ);
        const FVector WaistEnd = WaistStart + (Forward * 150.0f);
        FHitResult BlockHit;
        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DropWaistBlockCheck), false);
        QueryParams.AddIgnoredActor(Player);

        const bool bBlocked =
            GetWorld()->SweepSingleByChannel(BlockHit, WaistStart, WaistEnd, FQuat::Identity, ECC_Visibility,
                                             FCollisionShape::MakeSphere(20.0f), QueryParams);

        if (bBlocked)
        {
            if (Player->HUDWidget)
            {
                Player->HUDWidget->ShowErrorMessage(FText::FromString(TEXT("앞에 장애물이 있습니다")));
            }
            return false;
        }

        const FVector SpawnLoc(BaseLoc.X, BaseLoc.Y, PlayerLoc.Z + 20.0f);

        AItem *DroppedItem =
            GetWorld()->SpawnActor<AItem>(Player->ItemClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
        if (!DroppedItem)
        {
            if (Player->HUDWidget)
            {
                Player->HUDWidget->ShowErrorMessage(FText::FromString(TEXT("아이템 스폰에 실패했습니다")));
            }
            return false;
        }

        DroppedItem->InitializeItem(Player->CurrentHeldItem);
        Player->CurrentHeldItem = nullptr;

        if (DropSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DropSound, Player->GetActorLocation());
        }

        if (Player->HUDWidget)
        {
            Player->HUDWidget->UpdateAcquisitionInfo(nullptr, FText::GetEmpty());
        }

        return true;
    }

    return false;
}

void UCPRActionComponent::ActionPause(bool IsPause)
{
    UGameplayStatics::SetGamePaused(GetWorld(), IsPause);
}
