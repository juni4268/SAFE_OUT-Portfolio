#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPRExtinguishable.h"
#include "CPREvent_FireElec.generated.h"

class ACPREvent_Generator;
class UBoxComponent;
class UAudioComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class PROJECT_CPR_API ACPREvent_FireElec
	: public ACPREventBase, public ICPRExtinguishable
{
	GENERATED_BODY()

public:
	ACPREvent_FireElec();

protected:
	virtual void BeginPlay() override;
	virtual void OnTimerExpired() override;
	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus) override;

	UFUNCTION()
	void OnElecBurnZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnNormalBurnZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireElec|Mesh")
	UStaticMeshComponent* EventMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FireElec|Link")
	ACPREvent_Generator* LinkedGenerator = nullptr;

	UFUNCTION(BlueprintCallable, Category = "FireElec")
	void SetElecBurnZoneActive(bool bActive);

	virtual bool ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireElec|Fire")
	float FireLevel = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireElec|Fire")
	bool bIsOnFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireElec|Fire")
	float FireTimeLimit = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireElec|Fire")
	FName AcceptedExtinguisherTag = FName("ExtinguisherCO2");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireElec|Fire")
	UBoxComponent* ElecBurnZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireElec|Fire")
	UBoxComponent* NormalBurnZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings TimeOutDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings ElecBurnedDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings NormalBurnedDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireElec|Sound")
	USoundBase* FireSound;

	UPROPERTY()
	UAudioComponent* FireAudioComp;

	UFUNCTION(BlueprintImplementableEvent, Category = "FireElec",
		meta = (DisplayName = "On Fire Started"))
	void ReceiveFireStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "FireElec",
		meta = (DisplayName = "On Fire Level Changed"))
	void ReceiveFireLevelChanged(float NewFireLevel);
    UFUNCTION(BlueprintImplementableEvent, Category = "FireElec", meta = (DisplayName = "On Elec Burned Death"))
    void ReceiveElecBurnedDeath();

private:

	FTimerHandle FireTimerHandle;

	bool bElecBurnZoneActive = true;

	void TriggerFire();
};
