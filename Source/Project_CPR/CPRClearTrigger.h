#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPREventBase.h"
#include "CPRClearTrigger.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UTexture2D;
class UCPRMissionWidget;

UCLASS()
class PROJECT_CPR_API ACPRClearTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACPRClearTrigger();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	UBoxComponent* TriggerZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clear|Marker")
	UStaticMeshComponent* MarkerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Marker")
	UTexture2D* MarkerTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Marker", meta = (ClampMin = "0.01"))
	float MarkerBaseScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Marker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PulseAmplitude = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Marker", meta = (ClampMin = "0.1"))
	float PulseSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Area")
	bool bShowClearAreaBorder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	bool bLockOnClear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear|Audio")
	FName AudioStopTag = FName("ClearStopAudio");

	UFUNCTION(BlueprintImplementableEvent, Category = "Clear", meta = (DisplayName = "On Clear Conditions Met"))
	void OnClearConditionsMet();

	UFUNCTION(BlueprintImplementableEvent, Category = "Clear", meta = (DisplayName = "On Level Cleared"))
	void ReceiveLevelCleared();

	UFUNCTION(BlueprintCallable, Category = "Clear")
	void LockForClear();

private:

	UFUNCTION()
	void OnMissionAllCompleted();

	bool TryBindMissionWidget();

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void HandleConditionsMet();

	void SuspendOverheatsForClear();

	void StopTaggedWorldAudio();

	void UpdateMarkerTexture();

	void TickMarker(float DeltaTime);

	bool bConditionsMet = false;

	bool bCleared = false;

	bool bMissionWidgetBound = false;

	UPROPERTY()
	UMaterialInstanceDynamic* MarkerMID = nullptr;

	float MarkerPulseTime = 0.0f;
};
