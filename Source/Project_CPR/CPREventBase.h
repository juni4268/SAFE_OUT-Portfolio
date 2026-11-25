#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPREventBase.generated.h"

class UAnimMontage;
class UTexture2D;
class ACPRCharacter;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

USTRUCT(BlueprintType)
struct PROJECT_CPR_API FMarkerAnimSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	float PulseAmplitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker", meta = (ClampMin = "0.1"))
	float PulseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	float RotationSpeed;

	FMarkerAnimSettings()
		: PulseAmplitude(0.0f)
		, PulseSpeed(1.0f)
		, RotationSpeed(0.0f)
	{
	}
};

USTRUCT(BlueprintType)
struct PROJECT_CPR_API FEventDeathSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	UAnimMontage* DeathAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	UTexture2D* DeathResultImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death", meta = (MultiLine = true))
	FText DeathResultText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	float ExtraDelayAfterAnim = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	float ImpulseStrength = 0.0f;
};

UENUM(BlueprintType)
enum class ECPREventStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Active UMETA(DisplayName = "Active"),
	Interacting UMETA(DisplayName = "Interacting"),
	Resolved UMETA(DisplayName = "Resolved"),
	Failed UMETA(DisplayName = "Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventStatusChanged, ECPREventStatus, NewStatus);

UCLASS()
class PROJECT_CPR_API ACPREventBase : public AActor
{
	GENERATED_BODY()

public:
	ACPREventBase();

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void SetEventStatus(ECPREventStatus NewStatus);

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void StartEvent(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Event")
	void ResumeAutoStartAfterGameplayStart();

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnEventStatusChanged OnEventStatusChanged;

	UFUNCTION(BlueprintPure, Category = "Event")
	bool IsInProgress() const
	{
		return CurrentStatus == ECPREventStatus::Active || CurrentStatus == ECPREventStatus::Interacting;
	}

	UFUNCTION(BlueprintPure, Category = "Event")
	bool IsResolved() const
	{
		return CurrentStatus == ECPREventStatus::Resolved;
	}

	UFUNCTION(BlueprintPure, Category = "Event")
	bool IsInteracting() const
	{
		return CurrentStatus == ECPREventStatus::Interacting;
	}

	UFUNCTION(BlueprintPure, Category = "Event")
	virtual bool IsBlockingLevelClear() const
	{

		return !IsResolved();
	}

	virtual UTexture2D* GetOverrideMarkerTexture() const
	{
		return nullptr;
	}

	virtual FMarkerAnimSettings* GetOverrideMarkerAnim() 
	{
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void ResolveEvent();

	UFUNCTION(BlueprintNativeEvent, Category = "Event", meta = (DisplayName = "Receive Interaction Validated"))
	void ReceiveInteractionValidated(ACPRCharacter* TargetCharacter);
	virtual void ReceiveInteractionValidated_Implementation(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "On Event Started"))
	void ReceiveEventStarted(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "On Event Resolved"))
	void ReceiveEventResolved();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "On Event Failed"))
	void ReceiveEventFailed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "On Event Time Out"))
	void ReceiveEventTimeOut();

	UFUNCTION(BlueprintNativeEvent, Category = "Event", meta = (DisplayName = "On Player Interaction"))
	void ReceivePlayerInteraction(ACPRCharacter* TargetCharacter);
	virtual void ReceivePlayerInteraction_Implementation(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintNativeEvent, Category = "Event", meta = (DisplayName = "On Player Interaction Released"))
	void ReceivePlayerInteractionReleased(ACPRCharacter* TargetCharacter);
	virtual void ReceivePlayerInteractionReleased_Implementation(ACPRCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "Fail Event"))
	virtual void OnTimerExpired();

	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "Reset Resolve"))
	void ResetResolve();

protected:
	virtual void BeginPlay() override;

	virtual void OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus);

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void HandleCustomDieSequence(const FEventDeathSettings& DeathSettings);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EventMarker")
	UStaticMeshComponent* MarkerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_Idle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_Active = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_Resolved = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_Failed = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker")
	UTexture2D* MarkerTexture_OnFire = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker", meta = (ClampMin = "0.01"))
	float MarkerBaseScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_Resolved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_Failed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventMarker|Animations")
	FMarkerAnimSettings Anim_OnFire;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	UBoxComponent* InteractionZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Death")
	FEventDeathSettings DefaultDeathSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Setup")
	FName RequiredItemTag = FName("None");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup")
	bool bUseAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup", meta = (EditCondition = "bUseAutoStart"))
	float AutoStartDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup")
	bool bDisableCollisionOnResolve = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup")
	bool bDestroyOnResolve = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup")
	bool bHasTimeLimit = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Setup", meta = (EditCondition = "bHasTimeLimit"))
	float TimeLimit = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event|Setup")
	bool bShowInteractionUI = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	ECPREventStatus CurrentStatus = ECPREventStatus::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	bool bIsPlayerInRange = false;

	void HandleDieSequence();

	UFUNCTION()
	virtual void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	ACPRCharacter* CachedCharacter;

	void UpdateMarkerTexture();

	void TickMarker(float DeltaTime);

	UPROPERTY()
	UMaterialInstanceDynamic* MarkerMID = nullptr;

	float MarkerPulseTime = 0.0f;

	float MarkerCurrentRotation = 0.0f;

	FTimerHandle EventTimerHandle;
	FTimerHandle AutoStartTimerHandle;

	void ScheduleAutoStart();
	bool IsGameplayStartBlocked() const;

	UFUNCTION()
	void AutoStartTrigger();

public:
	virtual void Tick(float DeltaTime) override;
};
