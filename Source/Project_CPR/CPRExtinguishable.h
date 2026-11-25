#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPRExtinguishable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCPRExtinguishable : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_CPR_API ICPRExtinguishable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Fire")
	bool ApplyExtinguisher(float Amount, FName ExtinguisherTag);
};
