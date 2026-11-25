#include "CPREvent_Generator.h"
#include "CPRCharacter.h"
#include "CPREvent_FireElec.h"
#include "CPRHUDWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACPREvent_Generator::ACPREvent_Generator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	LeverDragAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("LeverDragAudioComp"));
	LeverDragAudioComp->SetupAttachment(RootComponent);
	LeverDragAudioComp->bAutoActivate = false;

	bHasTimeLimit = false;
	bUseAutoStart = false;
}

void ACPREvent_Generator::BeginPlay()
{
	Super::BeginPlay();

	if (LeverDragAudioComp && LeverDragSound)
	{
		LeverDragAudioComp->SetSound(LeverDragSound);
	}
}

void ACPREvent_Generator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LeverDragAudioComp && LeverDragAudioComp->IsPlaying())
	{
		if (GetWorld()->GetTimeSeconds() - LastDragTime > 0.15f)
		{
			LeverDragAudioComp->Stop();
		}
	}
}

void ACPREvent_Generator::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{
	Super::OnStatusChanged(OldStatus, NewStatus);

	if ((NewStatus == ECPREventStatus::Failed || NewStatus == ECPREventStatus::Resolved) && OldStatus == ECPREventStatus::Interacting)
	{
		EndLeverInteraction();
	}
}

void ACPREvent_Generator::DetectAndInteract(ACPRCharacter* TargetCharacter)
{
	if (CurrentStatus != ECPREventStatus::Active || !TargetCharacter || !TargetCharacter->FollowCamera)
	{
		return;
	}

	if (!bDoorOpen)
	{
		InteractWithDoor(TargetCharacter);
		return;
	}

	FVector Start = TargetCharacter->FollowCamera->GetComponentLocation();
	FVector End = Start + (TargetCharacter->FollowCamera->GetForwardVector() * 600.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TargetCharacter);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		UPrimitiveComponent* HitComp = HitResult.GetComponent();
		if (HitComp && HitComp->ComponentHasTag(TEXT("Lever")))
		{
			InteractWithLever(TargetCharacter);
			return;
		}
	}

	InteractWithDoor(TargetCharacter);
}

void ACPREvent_Generator::InteractWithDoor(ACPRCharacter* TargetCharacter)
{
	if (CurrentStatus != ECPREventStatus::Active || !TargetCharacter)
	{
		return;
	}

	bDoorOpen = !bDoorOpen;
	ReceiveDoorStateChanged(bDoorOpen);
}

void ACPREvent_Generator::InteractWithLever(ACPRCharacter* TargetCharacter)
{
	if (!bDoorOpen || CurrentStatus != ECPREventStatus::Active || !TargetCharacter)
	{
		return;
	}

	const bool bHasGlove = TargetCharacter->CurrentHeldItem && TargetCharacter->CurrentHeldItem->ItemTag == GloveItemTag;
	if (!bHasGlove)
	{

		ReceiveElectrocuted();
		SetEventStatus(ECPREventStatus::Failed);
		HandleDieSequence();
		return;
	}

	APlayerController* PC = Cast<APlayerController>(TargetCharacter->GetController());
	if (!PC)
	{
		return;
	}

	CachedCharacter = TargetCharacter;
	SetEventStatus(ECPREventStatus::Interacting);
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
	EnableInput(PC);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->ClearActionBindings();
		if (LeverDragAction)
		{
			EIC->BindAction(LeverDragAction, ETriggerEvent::Triggered, this, &ACPREvent_Generator::OnLeverDrag);
		}
		if (LeverCancelAction)
		{
			EIC->BindAction(LeverCancelAction, ETriggerEvent::Triggered, this, &ACPREvent_Generator::OnLeverCancel);
		}
	}

	if (LeverMappingContext)
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				Subsystem->AddMappingContext(LeverMappingContext, 1);
			}
		}
	}

	ReceiveLeverInteractionStarted(TargetCharacter);

	TargetCharacter->SetCharacterVisibility(false, true);
	if (TargetCharacter->HUDWidget)
	{
		TargetCharacter->HUDWidget->SetGameplayHUDVisible(false);
		TargetCharacter->HUDWidget->ShowGeneratorOverlay(true);
		TargetCharacter->HUDWidget->ShowExitHint(true);
	}
}

void ACPREvent_Generator::EndLeverInteraction()
{
	if (!CachedCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController());
	if (PC)
	{
		if (LeverMappingContext)
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
				{
					Subsystem->RemoveMappingContext(LeverMappingContext);
				}
			}
		}
		DisableInput(PC);
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}

	CachedCharacter->SetCharacterVisibility(true, true);
	if (CachedCharacter->HUDWidget)
	{
		CachedCharacter->HUDWidget->ShowGeneratorOverlay(false);
		CachedCharacter->HUDWidget->SetGameplayHUDVisible(true);
		CachedCharacter->HUDWidget->ShowExitHint(false);
	}

	if (LeverDragAudioComp && LeverDragAudioComp->IsPlaying())
	{
		LeverDragAudioComp->Stop();
	}

	if (CurrentStatus == ECPREventStatus::Interacting)
	{
		SetEventStatus(ECPREventStatus::Active);
	}
	ReceiveLeverInteractionEnded();
}

void ACPREvent_Generator::OnLeverDrag(const FInputActionValue& Value)
{
	if (CurrentStatus != ECPREventStatus::Interacting)
	{
		return;
	}

	const float Delta = Value.Get<float>();
	if (FMath::Abs(Delta) > 0.001f)
	{
		LeverProgress = FMath::Clamp(LeverProgress + (Delta * LeverDragSensitivity * 0.01f), 0.0f, 1.0f);
		ReceiveLeverDragged(LeverProgress);

		if (LeverDragAudioComp && !LeverDragAudioComp->IsPlaying() && LeverDragAudioComp->Sound)
		{
			LeverDragAudioComp->Play();
		}
		LastDragTime = GetWorld()->GetTimeSeconds();
	}

	if (LeverProgress >= 1.0f)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->ClearActionBindings();
		}
		ReceiveGeneratorFixed();

		if (LinkedFireEvent)
		{
			LinkedFireEvent->SetElecBurnZoneActive(false);
		}
		ResolveEvent();
	}
}

void ACPREvent_Generator::OnLeverCancel(const FInputActionValue& Value)
{
	EndLeverInteraction();
}
