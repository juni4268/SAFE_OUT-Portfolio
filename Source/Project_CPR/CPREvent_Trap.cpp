#include "CPREvent_Trap.h"
#include "CPRCharacter.h"
#include "Components/BoxComponent.h"
#include "ItemData.h"

ACPREvent_Trap::ACPREvent_Trap()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACPREvent_Trap::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionZoneBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		if (CurrentStatus == ECPREventStatus::Idle)
		{
			StartEvent(Character);
		}
	}
}
