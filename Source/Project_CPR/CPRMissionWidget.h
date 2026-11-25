#pragma once

#include "Blueprint/UserWidget.h"
#include "CPREventBase.h"
#include "CoreMinimal.h"
#include "CPRMissionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMissionsCompleted);

class UImage;
class UTextBlock;
class ACPREvent_Overheat;
class ACPREvent_FireLithium;
class ACPREvent_ExplosivesZone;
class ACPREvent_Trap;

UCLASS()
class PROJECT_CPR_API UCPRMissionWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RefreshAllStatuses();

    UFUNCTION(BlueprintPure, Category = "Mission")
    bool IsAllMissionsCompleted() const { return bAllMissionsCompleted; }

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnAllMissionsCompleted OnAllMissionsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Sound")
    class USoundBase* MissionCompleteSound;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Valve_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Valve_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_FireElec_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_FireElec_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_AirCon_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_AirCon_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Overheat_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Overheat_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Lithium_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Lithium_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock *Text_Lithium;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Trap_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Trap_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock *Text_Trap;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Explosives_Resolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage *Img_Explosives_Unresolved;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock *Text_Explosives;

private:

    UPROPERTY()
    TArray<ACPREventBase *> ValveEvents;

    UPROPERTY()
    TArray<ACPREventBase *> FireElecEvents;

    UPROPERTY()
    TArray<ACPREventBase *> AirConEvents;

    UPROPERTY()
    TArray<ACPREvent_Overheat *> OverheatEvents;

    UPROPERTY()
    TArray<ACPREvent_FireLithium *> LithiumEvents;

    UPROPERTY()
    TArray<ACPREvent_Trap *> TrapEvents;

    UPROPERTY()
    TArray<ACPREvent_ExplosivesZone *> ExplosivesEvents;

    UPROPERTY()
    TArray<class ACPREventSpawner *> Spawners;

    UPROPERTY()
    TArray<ACPREventBase *> AllTrackedEvents;

    UPROPERTY()
    TArray<ACPREvent_Trap *> CountedResolvedTrapEvents;

    int32 ResolvedTrapCount = 0;

    void CollectEvents();

    UFUNCTION()
    void OnAnyEventStatusChanged(ECPREventStatus NewStatus);

    UFUNCTION()
    void OnSpawnerSpawnedEvent(class ACPREventBase* SpawnedEvent);

    void SetTypeImages(UImage *ResolvedImg, UImage *UnresolvedImg, bool bResolved);

    template<typename T>
    bool AreAllResolved(const TArray<T *> &Events) const
    {
        if (Events.Num() == 0)
        {
            return false;
        }

        for (T *Event : Events)
        {
            if (!IsValid(Event) || !Event->IsResolved())
            {
                return false;
            }
        }
        return true;
    }

    bool AreAllOverheatsStable() const;

    FTimerHandle CountRefreshTimerHandle;

    bool bAllMissionsCompleted = false;
};
