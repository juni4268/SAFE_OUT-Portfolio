#include "CPRPauseWidget.h"
#include "CPRCharacter.h"
#include "CPRTodoZoneTrigger.h"
#include "CPREventBase.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCPRPauseWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PauseMenu)
    {
        PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (Btn_Continue)
    {
        Btn_Continue->OnClicked.AddDynamic(this, &UCPRPauseWidget::OnContinueClicked);
    }
    if (Btn_Restart)
    {
        Btn_Restart->OnClicked.AddDynamic(this, &UCPRPauseWidget::OnRestartClicked);
    }
    if (Btn_Quit)
    {
        Btn_Quit->OnClicked.AddDynamic(this, &UCPRPauseWidget::OnQuitClicked);
    }
}

void UCPRPauseWidget::ShowPauseMenu(bool bVisible)
{
    bIsPauseMenuVisible = bVisible;

    if (PauseMenu)
    {
        PauseMenu->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        if (bVisible)
        {
            PC->SetShowMouseCursor(true);
            PC->SetInputMode(FInputModeGameAndUI());
            UGameplayStatics::SetGamePaused(GetWorld(), true);
        }
        else
        {
            bool bSkipMouseReset = false;
            bool bSkipPauseReset = false;

            if (ACPRCharacter* Character = Cast<ACPRCharacter>(PC->GetPawn()))
            {
                if (Character->ActiveTodoZone && Character->ActiveTodoZone->IsOpen())
                {

                    bSkipMouseReset = true;
                    bSkipPauseReset = true;
                }
                else if (Character->ActiveEvent && Character->ActiveEvent->IsInteracting())
                {

                    bSkipMouseReset = true;
                    bSkipPauseReset = false;
                }
            }

            if (!bSkipMouseReset)
            {
                PC->SetShowMouseCursor(false);
                PC->SetInputMode(FInputModeGameOnly());
            }

            if (!bSkipPauseReset)
            {
                UGameplayStatics::SetGamePaused(GetWorld(), false);
            }
        }
    }
}

void UCPRPauseWidget::OnContinueClicked()
{
    ShowPauseMenu(false);
}

void UCPRPauseWidget::OnRestartClicked()
{

    UGameplayStatics::SetGamePaused(GetWorld(), false);

    UWorld* World = GetWorld();
    if (World)
    {
        FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(World);
        UGameplayStatics::OpenLevel(World, FName(*CurrentLevel));
    }
}

void UCPRPauseWidget::OnQuitClicked()
{
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}
