#include "CPRTitleUI.h"
#include "CPRCutsceneUI.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Widget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Components/AudioComponent.h"

bool UCPRTitleUI::bHasPlayedPreIntroThisSession = false;

void UCPRTitleUI::NativeConstruct()
{
    Super::NativeConstruct();

    if (TitleMediaPlayer)
    {
        TitleMediaPlayer->OnEndReached.RemoveDynamic(this, &UCPRTitleUI::OnVideoEndReached);
        TitleMediaPlayer->OnEndReached.AddDynamic(this, &UCPRTitleUI::OnVideoEndReached);
    }

    if (OL_Ctrl)
    {
        OL_Ctrl->SetVisibility(ESlateVisibility::Hidden);
    }

    if (Btn_Start)
    {
        Btn_Start->OnClicked.RemoveDynamic(this, &UCPRTitleUI::OnClickStart);
        Btn_Start->OnClicked.AddDynamic(this, &UCPRTitleUI::OnClickStart);
    }

    if (Btn_Cinematic)
    {
        Btn_Cinematic->OnClicked.RemoveDynamic(this, &UCPRTitleUI::OnClickCinematic);
        Btn_Cinematic->OnClicked.AddDynamic(this, &UCPRTitleUI::OnClickCinematic);
    }

    if (Btn_Controls)
    {
        Btn_Controls->OnClicked.RemoveDynamic(this, &UCPRTitleUI::OnClickControls);
        Btn_Controls->OnClicked.AddDynamic(this, &UCPRTitleUI::OnClickControls);
    }

    if (Btn_Quit)
    {
        Btn_Quit->OnClicked.RemoveDynamic(this, &UCPRTitleUI::OnClickQuit);
        Btn_Quit->OnClicked.AddDynamic(this, &UCPRTitleUI::OnClickQuit);
    }

    if (Btn_Exit)
    {
        Btn_Exit->OnClicked.RemoveDynamic(this, &UCPRTitleUI::OnClickExit);
        Btn_Exit->OnClicked.AddDynamic(this, &UCPRTitleUI::OnClickExit);
    }

    PlayInitialTitleVideo();
}

void UCPRTitleUI::OnClickStart()
{
    if (!GameLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(GetWorld(), GameLevelName);
    }
}

void UCPRTitleUI::OnClickCinematic()
{
    StopBGM();

    if (!PlayPreIntroVideo())
    {
        if (!PlayTitleVideo(IntroVideo, ETitleVideoState::Intro))
        {
            PlayLoopVideo();
        }
    }
}

void UCPRTitleUI::OnClickControls()
{
    if (OL_Ctrl)
    {
        OL_Ctrl->SetVisibility(ESlateVisibility::Visible);
    }

    if (Vbox_Menu)
    {
        Vbox_Menu->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UCPRTitleUI::OnClickQuit()
{
    if (OL_Ctrl)
    {
        OL_Ctrl->SetVisibility(ESlateVisibility::Hidden);
    }

    if (Vbox_Menu)
    {
        Vbox_Menu->SetVisibility(ESlateVisibility::Visible);
    }
}

void UCPRTitleUI::OnClickExit()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PC)
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCPRTitleUI::OnPreIntroCutsceneEnded(FName CutsceneTag)
{
    UE_LOG(LogTemp, Log, TEXT("TitleUI: PreIntro ended. Tag=%s, Active=%s, IntroVideo=%s, TitleMediaPlayer=%s"),
        *CutsceneTag.ToString(),
        bIsPreIntroCutsceneActive ? TEXT("True") : TEXT("False"),
        IntroVideo ? *IntroVideo->GetName() : TEXT("Null"),
        TitleMediaPlayer ? TEXT("Valid") : TEXT("Null"));

    if (!bIsPreIntroCutsceneActive)
    {
        return;
    }

    bIsPreIntroCutsceneActive = false;
    bHasPlayedPreIntroThisSession = true;

    UCPRCutsceneUI* FinishedCutsceneUI = ActiveCutsceneUI;
    ActiveCutsceneUI = nullptr;

    if (FinishedCutsceneUI)
    {
        FinishedCutsceneUI->OnCutsceneEnded.RemoveDynamic(this, &UCPRTitleUI::OnPreIntroCutsceneEnded);
        FinishedCutsceneUI->RemoveFromParent();
    }

    if (!PlayTitleVideo(IntroVideo, ETitleVideoState::Intro))
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleUI: Failed to play Intro after PreIntro. Playing Loop fallback."));
        PlayLoopVideo();
    }
}

void UCPRTitleUI::OnVideoEndReached()
{
    if (bIsPreIntroCutsceneActive)
    {
        return;
    }

    if (!TitleMediaPlayer)
    {
        return;
    }

    if (CurrentVideoState == ETitleVideoState::PreIntro)
    {
        bHasPlayedPreIntroThisSession = true;

        if (PlayTitleVideo(IntroVideo, ETitleVideoState::Intro))
        {
            return;
        }

        PlayLoopVideo();
        return;
    }

    if (CurrentVideoState == ETitleVideoState::Intro)
    {
        PlayLoopVideo();
        return;
    }

    PlayLoopVideo();
}

void UCPRTitleUI::PlayLoopBGM()
{

    if (bIsPreIntroCutsceneActive)
    {
        return;
    }

    if (CurrentVideoState != ETitleVideoState::Loop)
    {
        PlayLoopVideo();
    }

    if (LoopBGM && (!BGMAudioComponent || !BGMAudioComponent->IsPlaying()))
    {
        BGMAudioComponent = UGameplayStatics::SpawnSound2D(this, LoopBGM);
        bIsLoopingState = true;
    }
}

void UCPRTitleUI::StopBGM()
{
    if (BGMAudioComponent && BGMAudioComponent->IsPlaying())
    {
        BGMAudioComponent->Stop();
    }
}

bool UCPRTitleUI::PlayTitleVideo(UMediaSource* Video, ETitleVideoState NewState)
{
    if (!TitleMediaPlayer || !Video)
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleUI: PlayTitleVideo failed. State=%d, Video=%s, TitleMediaPlayer=%s"),
            static_cast<int32>(NewState),
            Video ? *Video->GetName() : TEXT("Null"),
            TitleMediaPlayer ? TEXT("Valid") : TEXT("Null"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("TitleUI: PlayTitleVideo. State=%d, Video=%s"),
        static_cast<int32>(NewState), *Video->GetName());

    CurrentVideoState = NewState;
    bIsLoopingState = NewState == ETitleVideoState::Loop;

    TitleMediaPlayer->OpenSource(Video);
    TitleMediaPlayer->Play();

    return true;
}

bool UCPRTitleUI::PlayPreIntroVideo()
{
    if (!PreIntroVideo || !CutsceneUIClass)
    {
        return false;
    }

    CurrentVideoState = ETitleVideoState::PreIntro;
    bIsPreIntroCutsceneActive = true;
    bIsLoopingState = false;

    if (TitleMediaPlayer)
    {
        TitleMediaPlayer->Pause();
    }

    if (ActiveCutsceneUI)
    {
        ActiveCutsceneUI->OnCutsceneEnded.RemoveDynamic(this, &UCPRTitleUI::OnPreIntroCutsceneEnded);
        ActiveCutsceneUI->SetVisibility(ESlateVisibility::Collapsed);
        ActiveCutsceneUI->RemoveFromParent();
        ActiveCutsceneUI = nullptr;
    }

    ActiveCutsceneUI = CreateWidget<UCPRCutsceneUI>(GetWorld(), CutsceneUIClass);
    if (!ActiveCutsceneUI)
    {
        bIsPreIntroCutsceneActive = false;
        return false;
    }

    ActiveCutsceneUI->OnCutsceneEnded.RemoveDynamic(this, &UCPRTitleUI::OnPreIntroCutsceneEnded);
    ActiveCutsceneUI->OnCutsceneEnded.AddDynamic(this, &UCPRTitleUI::OnPreIntroCutsceneEnded);
    ActiveCutsceneUI->AddToViewport(CutsceneZOrder);
    ActiveCutsceneUI->PlayCutscene(PreIntroVideo, TEXT("PreIntro"));

    return true;
}

void UCPRTitleUI::PlayInitialTitleVideo()
{
    if (!bHasPlayedPreIntroThisSession && PlayPreIntroVideo())
    {
        return;
    }

    if (PlayTitleVideo(IntroVideo, ETitleVideoState::Intro))
    {
        return;
    }

    PlayLoopVideo();
}

void UCPRTitleUI::PlayLoopVideo()
{
    if (PlayTitleVideo(LoopVideo, ETitleVideoState::Loop))
    {
        PlayLoopBGM();
    }
}
