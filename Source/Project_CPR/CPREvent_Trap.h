#pragma once

#include "CPREventBase.h"
#include "CoreMinimal.h"
#include "CPREvent_Trap.generated.h"

UCLASS()
class PROJECT_CPR_API ACPREvent_Trap : public ACPREventBase
{
	GENERATED_BODY()

public:
	ACPREvent_Trap();

protected:

	virtual void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
