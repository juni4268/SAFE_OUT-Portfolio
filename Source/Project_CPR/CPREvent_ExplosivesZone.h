#pragma once

#include "CoreMinimal.h"
#include "CPREventBase.h"
#include "CPREvent_ExplosivesZone.generated.h"

class UBoxComponent;
class AItem;

UCLASS()
class PROJECT_CPR_API ACPREvent_ExplosivesZone : public ACPREventBase
{
    GENERATED_BODY()

public:
    ACPREvent_ExplosivesZone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExplosivesZone")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExplosivesZone")
    int32 RequiredCount = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExplosivesZone")
    FName ExplosivesItemTag = FName("Explosives");

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExplosivesZone")
    UBoxComponent *ItemDetectionZone;

    UFUNCTION(BlueprintImplementableEvent, Category = "ExplosivesZone", meta = (DisplayName = "On Count Changed"))
    void ReceiveCountChanged(int32 NewCount, int32 MaxCount);

private:
    FTimerHandle InitMaxCountTimerHandle;

    UFUNCTION()
    void InitMaxCount();

    UFUNCTION()
    void OnItemZoneBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                const FHitResult &SweepResult);

    UFUNCTION()
    void OnItemZoneEndOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                              UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

    void RecalculateCount();
};
