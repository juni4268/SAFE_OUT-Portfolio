#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPREventTimerActor.generated.h"

UCLASS()
class PROJECT_CPR_API ACPREventTimerActor : public ACPREventBase
{
	GENERATED_BODY()

public:
	ACPREventTimerActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	float InitialDelay = 5.0f;

	void OnInitialDelayExpired();
};
