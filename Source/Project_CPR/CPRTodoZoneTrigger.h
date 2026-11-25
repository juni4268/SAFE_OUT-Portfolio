#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPRTodoZoneTrigger.generated.h"

class UBoxComponent;
class ACPRCharacter;

UCLASS()
class PROJECT_CPR_API ACPRTodoZoneTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACPRTodoZoneTrigger();

	UFUNCTION(BlueprintCallable, Category = "TodoZone")
	void CloseTodoZone();

	void OpenTodoZone();

	bool IsOpen() const { return bIsOpen; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TodoZone")
	UBoxComponent* TriggerZone;

	UFUNCTION(BlueprintImplementableEvent, Category = "TodoZone", meta = (DisplayName = "On TodoZone Opened"))
	void OnTodoZoneOpenedBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "TodoZone", meta = (DisplayName = "On TodoZone Closed"))
	void OnTodoZoneClosedBP();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TodoZone|Input")
	class UInputMappingContext* TodoMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TodoZone|Input")
	class UInputAction* QuitAction;

private:

	UPROPERTY()
	ACPRCharacter* CachedCharacter = nullptr;

	bool bIsOpen = false;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void RegisterInteraction(ACPRCharacter* Character);

	void UnregisterInteraction(ACPRCharacter* Character);

	void OnTodoQuit(const struct FInputActionValue& Value);
};
