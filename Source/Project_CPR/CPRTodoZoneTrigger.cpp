#include "CPRTodoZoneTrigger.h"

#include "CPRCharacter.h"
#include "CPRHUDWidget.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ACPRTodoZoneTrigger::ACPRTodoZoneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	SetRootComponent(TriggerZone);

	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ACPRTodoZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPRTodoZoneTrigger::OnTriggerBeginOverlap
	);

	TriggerZone->OnComponentEndOverlap.AddDynamic(
		this,
		&ACPRTodoZoneTrigger::OnTriggerEndOverlap
	);
}

void ACPRTodoZoneTrigger::CloseTodoZone()
{
	if (!bIsOpen || !CachedCharacter)
	{
		return;
	}

	bIsOpen = false;

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (CachedCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
		{
			if (TodoMappingContext)
			{
				if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
					{
						Subsystem->RemoveMappingContext(TodoMappingContext);
					}
				}
			}
			DisableInput(PC);

			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}

		if (CachedCharacter->HUDWidget)
		{
			CachedCharacter->HUDWidget->SetGameplayHUDVisible(true);
			CachedCharacter->HUDWidget->ShowExitHint(false);
		}
	}

	OnTodoZoneClosedBP();
}

void ACPRTodoZoneTrigger::OnTodoQuit(const FInputActionValue& Value)
{
	CloseTodoZone();
}

void ACPRTodoZoneTrigger::OpenTodoZone()
{
	if (bIsOpen || !CachedCharacter)
	{
		return;
	}

	bIsOpen = true;

	UGameplayStatics::SetGamePaused(GetWorld(), true);

	if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
	{
		EnableInput(PC);

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->ClearActionBindings();
			if (QuitAction)
			{
				EIC->BindAction(QuitAction, ETriggerEvent::Triggered, this, &ACPRTodoZoneTrigger::OnTodoQuit);
			}
		}

		if (TodoMappingContext)
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
				{
					Subsystem->AddMappingContext(TodoMappingContext, 1);
				}
			}
		}

		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}

	if (CachedCharacter->HUDWidget)
	{
		CachedCharacter->HUDWidget->SetGameplayHUDVisible(false);
		CachedCharacter->HUDWidget->ShowExitHint(true);
	}

	OnTodoZoneOpenedBP();
}

void ACPRTodoZoneTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	CachedCharacter = Character;

	Character->ActiveTodoZone = this;

	Character->UpdateInteractUI();
}

void ACPRTodoZoneTrigger::OnTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor);
	if (!Character || Character->ActiveTodoZone != this)
	{
		return;
	}

	if (bIsOpen)
	{
		CloseTodoZone();
	}

	Character->ActiveTodoZone = nullptr;
	CachedCharacter = nullptr;

	Character->UpdateInteractUI();
}
