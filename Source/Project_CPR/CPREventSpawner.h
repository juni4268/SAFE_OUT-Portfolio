#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPREventSpawner.generated.h"

class ACPREventBase;

USTRUCT(BlueprintType)
struct FEventSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner")
	TSubclassOf<ACPREventBase> EventClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner", meta = (ClampMin = "1"))
	int32 SpawnCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner")
	FName SpawnPointTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner|Delay", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SpawnDelayMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner|Delay", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SpawnDelayMax = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventSpawned, class ACPREventBase*, SpawnedEvent);

UCLASS()
class PROJECT_CPR_API ACPREventSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACPREventSpawner();

	UPROPERTY(BlueprintAssignable, Category = "Event Spawner")
	FOnEventSpawned OnEventSpawned;

	UFUNCTION(BlueprintPure, Category = "Event Spawner")
	int32 GetPlannedSpawnCount(TSubclassOf<ACPREventBase> TargetClass) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Spawner")
	TArray<FEventSpawnInfo> EventRoster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event Spawner")
	TArray<ACPREventBase*> SpawnedEvents;

private:

	TArray<FVector> UsedLocations;

	bool IsLocationUsed(const FVector& Location) const;

	UFUNCTION()
	void SpawnEventDeferred(TSubclassOf<ACPREventBase> EventClass, FVector SpawnLoc, FRotator SpawnRot);
};
