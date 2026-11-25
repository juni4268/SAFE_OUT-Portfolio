#include "CPREvent_Valve.h"
#include "CPRCharacter.h"
#include "CPRHUDWidget.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"

ACPREvent_Valve::ACPREvent_Valve()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ValveTurnAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ValveTurnAudioComp"));
	ValveTurnAudioComp->SetupAttachment(RootComponent);
	ValveTurnAudioComp->bAutoActivate = false;

	bHasTimeLimit = false; 
	bUseAutoStart = true;
	AutoStartDelay = 5.0f;
}

void ACPREvent_Valve::BeginPlay()
{
	Super::BeginPlay();
}

void ACPREvent_Valve::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{
	Super::OnStatusChanged(OldStatus, NewStatus);

	if (NewStatus == ECPREventStatus::Failed || NewStatus == ECPREventStatus::Resolved)
	{
		EndValveInteraction();
	}
}

void ACPREvent_Valve::OnTimerExpired()
{
	if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
	{
		return;
	}

	SetEventStatus(ECPREventStatus::Failed);
	HandleCustomDieSequence(TimeOutDeathSettings);
}

void ACPREvent_Valve::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ValveTurnAudioComp && ValveTurnAudioComp->IsPlaying())
	{
		if (GetWorld()->GetTimeSeconds() - LastTurnTime > 0.15f)
		{
			ValveTurnAudioComp->Stop();
		}
	}

	if (IsInProgress())
	{
		CurrentPressure += PressureIncreaseRate * DeltaTime;

		if (CurrentPressure >= MaxPressure)
		{
			CurrentPressure = MaxPressure;
			ReceivePressureChanged(CurrentPressure, MaxPressure);
			OnTimerExpired();
		}
		else
		{
			ReceivePressureChanged(CurrentPressure, MaxPressure);
		}
	}
}

void ACPREvent_Valve::StartValveInteraction(ACPRCharacter* TargetCharacter)
{
	if (CurrentStatus != ECPREventStatus::Active || !TargetCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(TargetCharacter->Controller);
	if (!PC)
	{
		return;
	}

	SetEventStatus(ECPREventStatus::Interacting);
	CachedCharacter = TargetCharacter;

	FVector ToValve = GetActorLocation() - TargetCharacter->GetActorLocation();
	FRotator LookAtRot = ToValve.Rotation();
	LookAtRot.Pitch = FMath::Clamp(LookAtRot.Pitch, -30.0f, 0.0f);
	LookAtRot.Roll = 0.0f;
	PC->SetControlRotation(LookAtRot);

	TargetCharacter->SetCharacterVisibility(false, true);
	if (TargetCharacter->HUDWidget)
	{
		TargetCharacter->HUDWidget->SetGameplayHUDVisible(false);
		TargetCharacter->HUDWidget->ShowValveOverlay(true);
		TargetCharacter->HUDWidget->ShowExitHint(true);
	}

	EnableInput(PC);
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->ClearActionBindings();
		if (TurnAction)
		{
			EIC->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ACPREvent_Valve::OnValveTurn);
		}
		if (QuitAction)
		{
			EIC->BindAction(QuitAction, ETriggerEvent::Triggered, this, &ACPREvent_Valve::OnValveQuit);
		}
	}

	if (ValveMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ValveMappingContext, 1);
		}
	}

	ReceiveValveInteractionStarted(TargetCharacter);
}

void ACPREvent_Valve::EndValveInteraction()
{
	if (CurrentStatus != ECPREventStatus::Interacting || !CachedCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(CachedCharacter->Controller);
	if (PC)
	{
		if (ValveMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(ValveMappingContext);
			}
		}
		DisableInput(PC);
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
	}

	CachedCharacter->SetCharacterVisibility(true, true);
	if (CachedCharacter->HUDWidget)
	{
		CachedCharacter->HUDWidget->ShowValveOverlay(false);
		CachedCharacter->HUDWidget->SetGameplayHUDVisible(true);
		CachedCharacter->HUDWidget->ShowExitHint(false);
	}

	if (ValveTurnAudioComp && ValveTurnAudioComp->IsPlaying())
	{
		ValveTurnAudioComp->Stop();
	}

	if (CurrentStatus == ECPREventStatus::Interacting)
	{
		SetEventStatus(ECPREventStatus::Active);
	}

	ReceiveValveInteractionEnded();
}

void ACPREvent_Valve::OnValveTurn(const FInputActionValue& Value)
{
	if (CurrentStatus != ECPREventStatus::Interacting)
	{
		return;
	}

	float TurnAxis = Value.Get<float>();
	if (FMath::Abs(TurnAxis) > 0.01f)
	{
		if (MaxSingleInputDelta > 0.0f && FMath::Abs(TurnAxis) >= MaxSingleInputDelta)
		{
			SetEventStatus(ECPREventStatus::Failed);
			HandleCustomDieSequence(OverturnDeathSettings);
			return;
		}

		ReceiveValveTurned(TurnAxis);
		CurrentPressure -= TurnAxis * MouseSensitivity;

		if (ValveTurnAudioComp && !ValveTurnAudioComp->IsPlaying() && ValveTurnAudioComp->Sound)
		{
			ValveTurnAudioComp->Play();
		}
		LastTurnTime = GetWorld()->GetTimeSeconds();

		if (CurrentPressure <= 0.0f)
		{
			CurrentPressure = 0.0f;
			ReceivePressureChanged(CurrentPressure, MaxPressure);
			if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
			{
				EIC->ClearActionBindings();
			}
			ReceiveValveSucceeded();
			return;
		}

		ReceivePressureChanged(CurrentPressure, MaxPressure);
	}
}

void ACPREvent_Valve::OnValveQuit(const FInputActionValue& Value)
{
	EndValveInteraction();
}
