#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPRExtinguishable.h"
#include "InputActionValue.h"
#include "CPREvent_Overheat.generated.h"

class ACPRCharacter;
class UInputMappingContext;
class UInputAction;
class UWidgetComponent;
class UCPROverheatWidget;

UENUM(BlueprintType)
enum class EOverheatPhase : uint8
{
	Managing UMETA(DisplayName = "Managing"),
	OnFire   UMETA(DisplayName = "OnFire")
};

UCLASS()
class PROJECT_CPR_API ACPREvent_Overheat : public ACPREventBase, public ICPRExtinguishable
{
	GENERATED_BODY()

public:
	ACPREvent_Overheat();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnTimerExpired() override;
	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus) override;

	virtual void ReceivePlayerInteraction_Implementation(ACPRCharacter* TargetCharacter) override;
	virtual void ReceivePlayerInteractionReleased_Implementation(ACPRCharacter* TargetCharacter) override;

	virtual void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION()
	virtual void OnBurnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:

	virtual bool IsBlockingLevelClear() const override;
	virtual UTexture2D* GetOverrideMarkerTexture() const override;
	virtual FMarkerAnimSettings* GetOverrideMarkerAnim() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilMax = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilMin = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilStartRangeMin = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilStartRangeMax = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilCurrent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilConsumeRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Oil")
	float OilFillRate = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Oil")
	FName OilItemTag = FName("Oil");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantMax = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantMin = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantStartRangeMin = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantStartRangeMax = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantCurrent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantConsumeRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Coolant")
	float CoolantFillRate = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Coolant")
	FName CoolantItemTag = FName("Coolant");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempMax = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempMin = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempCurrent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempRiseRateOneEmpty = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempRiseBothEmpty = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature")
	float TempDecreaseRate = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Temperature", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TempDecreaseThresholdRatio = 0.3f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Overheat|Fire")
	float FireLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Fire")
	FName ExtinguisherItemTag = FName("Extinguisher");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|Fire")
	class UBoxComponent* BurnZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings TimeOutDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings BurnedDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_Overheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_Overheat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|State")
	EOverheatPhase CurrentPhase = EOverheatPhase::Managing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|State")
	bool bIsOnFire = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|State")
	bool bSuspendedForClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Animation")
	class UAnimMontage* OilFillAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Animation")
	class UAnimMontage* CoolantFillAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Sound")
	class USoundBase* HighTempWarningSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Sound")
	class USoundBase* OilFillSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Sound")
	class USoundBase* CoolantFillSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheat|Sound")
	class USoundBase* FireSound;

	UPROPERTY()
	class UAudioComponent* WarningAudioComp;

	UPROPERTY()
	class UAudioComponent* OilFillAudioComp;

	UPROPERTY()
	class UAudioComponent* CoolantFillAudioComp;

	UPROPERTY()
	class UAudioComponent* FireAudioComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|UI")
	UWidgetComponent* OverheatWidgetComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat|UI")
	UWidgetComponent* OverheatWidgetComp_Back;

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void StartOverheatInteraction(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void EndOverheatInteraction();

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void StartFillingOil(ACPRCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void StopFillingOil();

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void StartFillingCoolant(ACPRCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void StopFillingCoolant();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Input")
	UInputMappingContext *OverheatMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Input")
	UInputAction *FillOilAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Input")
	UInputAction *FillCoolantAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overheat|Input")
	UInputAction *QuitOverheatAction;

	virtual bool ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag) override;

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void DecreaseFireLevel(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Overheat")
	void SuspendForClear();

protected:
	bool bIsFillingOil = false;
	bool bIsFillingCoolant = false;

	bool bOverheatMarkerShown = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Overheat", meta = (DisplayName = "On Fire Level Changed"))
	void ReceiveFireLevelChanged(float NewFireLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Overheat", meta = (DisplayName = "On Overheat Started"))
	void ReceiveOverheatStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Overheat", meta = (DisplayName = "On Overheat Burned Death"))
	void ReceiveOverheatBurnedDeath();

private:
	void TriggerOverheat();
	void DoFillOilTick(float DeltaTime);
	void DoFillCoolantTick(float DeltaTime);
	void UpdateOverheatUI();

	void OnInputFillOilStarted(const FInputActionValue &Value);
	void OnInputFillOilEnded(const FInputActionValue &Value);
	void OnInputFillCoolantStarted(const FInputActionValue &Value);
	void OnInputFillCoolantEnded(const FInputActionValue &Value);
	void OnInputQuitOverheat(const FInputActionValue &Value);
};
