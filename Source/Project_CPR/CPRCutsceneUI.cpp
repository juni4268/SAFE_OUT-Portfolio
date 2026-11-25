// Fill out your copyright notice in the Description page of Project Settings.

#include "CPRCutsceneUI.h"
#include "Components/Button.h"
#include "MediaPlayer.h"
#include "MediaSource.h"

void UCPRCutsceneUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (MediaPlayer != nullptr)
	{

		MediaPlayer->OnEndReached.RemoveDynamic(this, &UCPRCutsceneUI::HandleCutsceneFinished);
		MediaPlayer->OnEndReached.AddDynamic(this, &UCPRCutsceneUI::HandleCutsceneFinished);
	}

	if (Btn_Skip != nullptr)
	{
		Btn_Skip->OnClicked.RemoveDynamic(this, &UCPRCutsceneUI::SkipCutscene);
		Btn_Skip->OnClicked.AddDynamic(this, &UCPRCutsceneUI::SkipCutscene);
	}
}

void UCPRCutsceneUI::PlayCutscene(UMediaSource* InMediaSource, FName CutsceneTag)
{
	if (MediaPlayer != nullptr && InMediaSource != nullptr)
	{
		bHasFinished = false;

		CurrentCutsceneTag = CutsceneTag;
		MediaSource = InMediaSource;

		MediaPlayer->OpenSource(MediaSource);
		MediaPlayer->Rewind();
		MediaPlayer->Play();

		OnCutsceneStartedBP();
	}
}

void UCPRCutsceneUI::SkipCutscene()
{
	if (MediaPlayer != nullptr)
	{
		MediaPlayer->Close();
	}

	HandleCutsceneFinished();
}

void UCPRCutsceneUI::HandleCutsceneFinished()
{
	if (bHasFinished)
	{
		return;
	}
	bHasFinished = true;

	UE_LOG(LogTemp, Log, TEXT("CutsceneUI: Finished. Tag=%s, DelegateBound=%s"),
		*CurrentCutsceneTag.ToString(),
		OnCutsceneEnded.IsBound() ? TEXT("True") : TEXT("False"));

	OnCutsceneFinishedBP();

	if (OnCutsceneEnded.IsBound())
	{
		OnCutsceneEnded.Broadcast(CurrentCutsceneTag);
	}
}
