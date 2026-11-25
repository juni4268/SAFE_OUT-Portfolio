#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "InputActionValue.h"
#include "CPREvent_Valve.generated.h"

class UInputMappingContext;
class UInputAction;
class ACPRCharacter;

UCLASS()
class PROJECT_CPR_API ACPREvent_Valve : public ACPREventBase
{
	GENERATED_BODY()

public:
	ACPREvent_Valve();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnTimerExpired() override;

	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus) override;

public:

	UFUNCTION(BlueprintCallable, Category = "Valve")
	void StartValveInteraction(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Valve")
	void EndValveInteraction();

protected:

	void OnValveTurn(const FInputActionValue& Value);
	void OnValveQuit(const FInputActionValue& Value);

	UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
	void ReceivePressureChanged(float NewPressure, float MaxValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
	void ReceiveValveTurned(float TurnDelta);

	UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
	void ReceiveValveSucceeded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Valve", meta = (DisplayName = "On Valve Interaction Started"))
	void ReceiveValveInteractionStarted(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Valve", meta = (DisplayName = "On Valve Interaction Ended"))
	void ReceiveValveInteractionEnded();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve|Settings")
	float MaxPressure = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve|Settings", meta = (ToolTip = "1초당 증가하는 압력량"))
	float PressureIncreaseRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve|Settings", meta = (ToolTip = "마우스 돌림에 따른 압력 감소 계수 (민감도)"))
	float MouseSensitivity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve|Settings", meta = (ToolTip = "한 프레임 입력 절댓값이 이 이상이면 즉시 폭발"))
	float MaxSingleInputDelta = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Valve|State")
	float CurrentPressure = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve|Input")
	UInputMappingContext* ValveMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve|Input")
	UInputAction* TurnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve|Input")
	UInputAction* QuitAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings TimeOutDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings OverturnDeathSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Valve|Sound")
	class UAudioComponent* ValveTurnAudioComp;

private:
	float LastTurnTime = 0.0f;
};
