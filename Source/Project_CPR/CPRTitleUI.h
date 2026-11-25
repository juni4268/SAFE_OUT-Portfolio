#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CPRTitleUI.generated.h"

class UButton;
class UWidget;
class UCPRCutsceneUI;

enum class ETitleVideoState : uint8
{
    None,
    PreIntro,
    Intro,
    Loop
};

UCLASS()
class PROJECT_CPR_API UCPRTitleUI : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Start;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Controls;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Quit;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Btn_Cinematic;

    UPROPERTY(meta = (BindWidget))
    UWidget* OL_Ctrl;

    UPROPERTY(meta = (BindWidget))
    UWidget* Vbox_Menu;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Exit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI")
    FName GameLevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Media")
    class UMediaPlayer* TitleMediaPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Media")
    class UMediaSource* PreIntroVideo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Media")
    class UMediaSource* IntroVideo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Media")
    class UMediaSource* LoopVideo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Cutscene")
    TSubclassOf<UCPRCutsceneUI> CutsceneUIClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Cutscene")
    int32 CutsceneZOrder = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleUI|Sound")
    class USoundBase* LoopBGM;

    UFUNCTION(BlueprintCallable, Category = "TitleUI|Sound")
    void PlayLoopBGM();

    UFUNCTION(BlueprintCallable, Category = "TitleUI|Sound")
    void StopBGM();

    UFUNCTION()
    void OnVideoEndReached();

    UFUNCTION()
    void OnPreIntroCutsceneEnded(FName CutsceneTag);

    UFUNCTION()
    void OnClickStart();

    UFUNCTION()
    void OnClickCinematic();

    UFUNCTION()
    void OnClickControls();

    UFUNCTION()
    void OnClickQuit();

    UFUNCTION()
    void OnClickExit();

private:

    bool bIsLoopingState = false;
    bool bIsPreIntroCutsceneActive = false;
    ETitleVideoState CurrentVideoState = ETitleVideoState::None;
    static bool bHasPlayedPreIntroThisSession;

    bool PlayTitleVideo(class UMediaSource* Video, ETitleVideoState NewState);
    bool PlayPreIntroVideo();
    void PlayInitialTitleVideo();
    void PlayLoopVideo();

    UPROPERTY()
    class UAudioComponent* BGMAudioComponent;

    UPROPERTY()
    UCPRCutsceneUI* ActiveCutsceneUI;
};
