#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPRGameFlowManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameplayStarted);

UCLASS()
class PROJECT_CPR_API ACPRGameFlowManager : public AActor
{
	GENERATED_BODY()

public:
	ACPRGameFlowManager();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	bool TryConsumeIntroCutsceneRequest(FName& OutCutsceneTag);

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void NotifyIntroCutsceneFinished();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGameplay();

	UFUNCTION(BlueprintPure, Category = "Game Flow")
	bool HasGameplayStarted() const
	{
		return bGameplayStarted;
	}

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnGameplayStarted OnGameplayStarted;

protected:
	virtual void BeginPlay() override;

private:
	void ResumeEventAutoStarts();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow", meta = (AllowPrivateAccess = "true"))
	bool bGameplayStarted = false;
};
