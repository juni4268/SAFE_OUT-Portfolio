// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "CPRCutsceneUI.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCutsceneEndedDelegate, FName, CutsceneTag);

UCLASS()
class PROJECT_CPR_API UCPRCutsceneUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void PlayCutscene(UMediaSource* InMediaSource, FName CutsceneTag);

	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void SkipCutscene();

	UPROPERTY(BlueprintAssignable, Category = "Cutscene")
	FOnCutsceneEndedDelegate OnCutsceneEnded;

protected:

	UFUNCTION()
	void HandleCutsceneFinished();

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Skip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	UMediaPlayer* MediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	UMediaSource* MediaSource;

	FName CurrentCutsceneTag;

	bool bHasFinished = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cutscene")
	void OnCutsceneStartedBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cutscene")
	void OnCutsceneFinishedBP();
};
