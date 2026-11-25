#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPREvent_AirCon.generated.h"

class AExponentialHeightFog;
class ACPRCharacter;

UCLASS()
class PROJECT_CPR_API ACPREvent_AirCon : public ACPREventBase
{
	GENERATED_BODY()

public:
	ACPREvent_AirCon();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Event|AirCon")
	void ClearFog();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|AirCon")
	void ReceiveFogUpdated(float DensityPercent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|AirCon|Setup")
	AExponentialHeightFog* TargetFogActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|AirCon|Settings")
	float DensityIncreaseRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|AirCon|Settings")
	float ClearSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|AirCon|Settings")
	float MaxDensityThreshold = 10.0f;

private:

	float CurrentDensity = 0.0f;

	bool bIsClearing = false;

	void UpdateFogVisuals();
};
