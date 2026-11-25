#pragma once

#include "CPREventBase.h"
#include "CoreMinimal.h"
#include "CPREvent_HardHat.generated.h"

class UStaticMeshComponent;

UCLASS()
class PROJECT_CPR_API ACPREvent_HardHat : public ACPREventBase
{
  GENERATED_BODY()

public:
  ACPREvent_HardHat();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  virtual void OnInteractionZoneBeginOverlap(
      UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
      const FHitResult &SweepResult) override;

  UFUNCTION()
  void OnFallingMeshHit(UPrimitiveComponent *HitComponent, AActor *OtherActor,
                        UPrimitiveComponent *OtherComp, FVector NormalImpulse,
                        const FHitResult &Hit);

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Event|HardHat|Setup")
  UStaticMeshComponent *FallingMesh;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|HardHat|Setup")
  float SpawnHeightOffset = 400.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|HardHat|Setup")
  float DestroyDelay = 5.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|HardHat|Sound")
  class USoundBase* HardHatHitSound;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|HardHat|Sound")
  class USoundBase* NoHardHatHitSound;

  UPROPERTY()
  AActor *TrackingTarget;

  bool bIsTracking = false;

  bool bHasFallen = false;
};
