#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPRExtinguishable.h"
#include "CPREvent_FireLithium.generated.h"

class ACPRCharacter;
class UBoxComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class PROJECT_CPR_API ACPREvent_FireLithium : public ACPREventBase, public ICPRExtinguishable
{
	GENERATED_BODY()

public:
	ACPREvent_FireLithium();

protected:
	virtual void BeginPlay() override;
	virtual void OnTimerExpired() override;
	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus) override;

	virtual void ReceivePlayerInteraction_Implementation(ACPRCharacter* TargetCharacter) override;
	virtual void ReceiveInteractionValidated_Implementation(ACPRCharacter* TargetCharacter) override;

	virtual void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION()
	virtual void OnBurnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:

	virtual UTexture2D* GetOverrideMarkerTexture() const override;
	virtual FMarkerAnimSettings* GetOverrideMarkerAnim() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FireLithium|Lithium")
	float LithiumCurrent = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Lithium")
	float LithiumMax = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Lithium")
	float LithiumPerInteraction = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FireLithium|Lithium")
	FName LithiumItemTag = FName("Lithium");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FireLithium|Fire")
	float FireLevel = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireLithium|Fire")
	bool bIsOnFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Fire")
	float FireTimeLimit = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Fire")
	FName AcceptedExtinguisherTag = FName("ExtinguisherD");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Fire")
	float FireTriggerDelay = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireLithium|Fire")
	UBoxComponent* BurnZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings TimeOutDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings BurnedDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLithium|Sound")
	USoundBase* FireSound;

	UPROPERTY()
	UAudioComponent* FireAudioComp;

	virtual bool ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FireLithium", meta = (DisplayName = "On Lithium Changed"))
	void ReceiveLithiumChanged(float NewValue, float MaxValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "FireLithium", meta = (DisplayName = "On Fire Started"))
	void ReceiveFireStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "FireLithium", meta = (DisplayName = "On Lithium Burned Death"))
	void ReceiveLithiumBurnedDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "FireLithium", meta = (DisplayName = "On Fire Level Changed"))
	void ReceiveFireLevelChanged(float NewFireLevel);

private:
	FTimerHandle FireDelayTimerHandle;

	void TriggerFire();
};
