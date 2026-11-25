#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CPRGameInstance.generated.h"

class UUserWidget;

UCLASS()
class PROJECT_CPR_API UCPRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void RequestPostLoadCutscene(FName CutsceneTag);

	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	bool ConsumePendingPostLoadCutscene(FName& OutCutsceneTag);

	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void NotifyPostLoadCutsceneFinished();

	UFUNCTION(BlueprintPure, Category = "Cutscene")
	bool HasPostLoadCutsceneRequest() const;

	UFUNCTION(BlueprintPure, Category = "Cutscene")
	bool IsGameplayStartBlocked() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading Screen")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading Screen", meta = (ClampMin = "0.0"))
	float MinimumLoadingScreenDisplayTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading Screen")
	FText LoadingText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading Screen")
	bool bShowLoadingScreenInEditor = false;

private:
	void BeginLoadingScreen(const FString& MapName);
	void EndLoadingScreen(UWorld* LoadedWorld);
	TSharedRef<class SWidget> CreateDefaultLoadingScreenWidget() const;

	UPROPERTY(Transient)
	FName PendingPostLoadCutsceneTag;

	UPROPERTY(Transient)
	bool bGameplayStartBlocked = false;
};
