#include "CPREventTimerActor.h"
#include "CPRCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACPREventTimerActor::ACPREventTimerActor()
{
}

void ACPREventTimerActor::BeginPlay()
{
	Super::BeginPlay();

	if (InitialDelay > 0.0f)
	{
		FTimerHandle InitialTimerHandle;
		GetWorldTimerManager().SetTimer(InitialTimerHandle, this, &ACPREventTimerActor::OnInitialDelayExpired, InitialDelay, false);
	}
	else
	{
		OnInitialDelayExpired();
	}
}

void ACPREventTimerActor::OnInitialDelayExpired()
{
	if (ACPRCharacter* Character = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		StartEvent(Character);
	}
}
