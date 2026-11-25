#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPRPlayerController.generated.h"

UCLASS()
class PROJECT_CPR_API ACPRPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
