#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "InputActionValue.h"
#include "CPREvent_Generator.generated.h"

class UInputMappingContext;
class UInputAction;
class USoundBase;
class UAudioComponent;
class ACPRCharacter;
class ACPREvent_FireElec;

UCLASS()
class PROJECT_CPR_API ACPREvent_Generator : public ACPREventBase
{
	GENERATED_BODY()

public:
	ACPREvent_Generator();

protected:
	virtual void BeginPlay() override;
	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus) override;
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Event|Generator")
	void DetectAndInteract(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Event|Generator")
	void InteractWithDoor(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Event|Generator")
	void InteractWithLever(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Event|Generator")
	void EndLeverInteraction();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Door State Changed"))
	void ReceiveDoorStateChanged(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Lever Dragged"))
	void ReceiveLeverDragged(float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Lever Interaction Started"))
	void ReceiveLeverInteractionStarted(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Lever Interaction Ended"))
	void ReceiveLeverInteractionEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Electrocuted"))
	void ReceiveElectrocuted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event|Generator", meta = (DisplayName = "On Generator Fixed"))
	void ReceiveGeneratorFixed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event|Generator|State")
	bool bDoorOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Generator|State")
	float LeverProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Generator|Settings")
	FName GloveItemTag = FName("Glove");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Generator|Settings", meta = (ClampMin = "0.01"))
	float LeverDragSensitivity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Generator|Input")
	UInputMappingContext* LeverMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Generator|Input")
	UInputAction* LeverDragAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Generator|Input")
	UInputAction* LeverCancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Generator|Sound")
	USoundBase* LeverDragSound;

	UPROPERTY()
	UAudioComponent* LeverDragAudioComp;

	float LastDragTime = 0.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Event|Generator|Link")
	ACPREvent_FireElec* LinkedFireEvent = nullptr;

private:
	void OnLeverDrag(const FInputActionValue& Value);
	void OnLeverCancel(const FInputActionValue& Value);
};
