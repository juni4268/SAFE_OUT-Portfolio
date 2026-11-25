#include "CPRHUDWidget.h"
#include "CPRCharacter.h"
#include "CPROverheatWidget.h"
#include "CPRTodoZoneTrigger.h"
#include "CPRMissionWidget.h"
#include "CPRPauseWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UCPRHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ItemError_Overlay)
    {
        ItemError_Overlay->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (InteractButton)
    {
        InteractButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (OL_Start)
    {
        OL_Start->SetVisibility(ESlateVisibility::Visible);
        GetWorld()->GetTimerManager().SetTimer(StartOverlayTimerHandle, this, &UCPRHUDWidget::HideStartOverlay, StartMessageDuration, false);
    }

    if (OL_Clear)
    {
        OL_Clear->SetVisibility(ESlateVisibility::Collapsed);
    }

    ShowValveOverlay(false);
    ShowGeneratorOverlay(false);

    if (Btn_ExitHint)
    {
        Btn_ExitHint->SetVisibility(ESlateVisibility::Collapsed);
        Btn_ExitHint->OnClicked.AddDynamic(this, &UCPRHUDWidget::OnExitHintClicked);
    }
}

void UCPRHUDWidget::OnExitHintClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ACPRCharacter* Character = Cast<ACPRCharacter>(PC->GetPawn()))
        {
            if (Character->ActiveTodoZone && Character->ActiveTodoZone->IsOpen())
            {
                Character->ActiveTodoZone->CloseTodoZone();
            }
        }
    }
}

void UCPRHUDWidget::UpdateInteractVisibility(bool bIsVisible)
{
    if (InteractButton)
    {
        InteractButton->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::SimulateInteractButtonPress(bool bIsPressed)
{
    if (InteractButton)
    {
        if (bIsPressed)
        {
            InteractButton->SetBackgroundColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        }
        else
        {
            InteractButton->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
}

void UCPRHUDWidget::UpdateAcquisitionInfo(UTexture2D *NewIcon, FText NewName)
{
    if (Item_Image)
    {
        Item_Image->SetBrushFromTexture(NewIcon);
    }

    if (Item_Text)
    {
        Item_Text->SetText(NewName);
    }

    OnItemAcquired();
}

void UCPRHUDWidget::ShowErrorMessage(FText Message, float DisplayTime)
{
    if (ItemError_Overlay && ItemError_Text)
    {
        ItemError_Text->SetText(Message);
        ItemError_Overlay->SetVisibility(ESlateVisibility::Visible);

        GetWorld()->GetTimerManager().ClearTimer(ItemErrorOverlayTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(ItemErrorOverlayTimerHandle, this, &UCPRHUDWidget::HideItemDropError,
                                               DisplayTime, false);
    }
}

void UCPRHUDWidget::HideItemDropError()
{
    if (ItemError_Overlay)
    {
        ItemError_Overlay->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::HideStartOverlay()
{
    if (OL_Start)
    {
        OL_Start->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::ShowOverheatPanel(bool bVisible, ACPREvent_Overheat *OwnerEvent)
{
    if (OverheatWidget)
    {
        OverheatWidget->ShowPanel(bVisible, OwnerEvent);
    }
}

void UCPRHUDWidget::UpdateOverheatStats(float OilRatio, float CoolantRatio, float TempRatio)
{
    if (OverheatWidget)
    {
        OverheatWidget->UpdateStats(OilRatio, CoolantRatio, TempRatio);
    }
}

void UCPRHUDWidget::SimulateOilButtonPress(bool bIsPressed)
{
    if (OverheatWidget)
    {
        OverheatWidget->SimulateOilButtonPress(bIsPressed);
    }
}

void UCPRHUDWidget::SimulateCoolantButtonPress(bool bIsPressed)
{
    if (OverheatWidget)
    {
        OverheatWidget->SimulateCoolantButtonPress(bIsPressed);
    }
}

void UCPRHUDWidget::ShowOverheatButtonBox(bool bVisible, ACPREvent_Overheat* Event)
{
    if (OverheatWidget)
    {
        OverheatWidget->ShowButtonBox(bVisible, Event);
    }
}

void UCPRHUDWidget::ShowPauseMenu(bool bVisible)
{
    if (PauseWidget)
    {
        PauseWidget->ShowPauseMenu(bVisible);
    }
}

void UCPRHUDWidget::TogglePauseMenu()
{
    if (PauseWidget)
    {
        bool bIsVisible = PauseWidget->IsPauseMenuVisible();
        ShowPauseMenu(!bIsVisible);
    }
}

UCPRMissionWidget* UCPRHUDWidget::GetMissionWidget() const
{
    return MissionWidget;
}

void UCPRHUDWidget::ShowClearText()
{
    if (OL_Clear)
    {
        OL_Clear->SetVisibility(ESlateVisibility::Visible);
    }

    OnClearTextShown();
}

void UCPRHUDWidget::SetGameplayHUDVisible(bool bVisible)
{

    if (bVisible)
    {

        if (InteractButton) InteractButton->SetVisibility(ESlateVisibility::Visible);
        if (ItemBox) ItemBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        if (MissionWidget) MissionWidget->SetVisibility(SavedMissionVisibility);
        if (OverheatWidget) OverheatWidget->SetVisibility(SavedOverheatVisibility);
        if (ItemError_Overlay) ItemError_Overlay->SetVisibility(SavedItemErrorVisibility);
    }
    else
    {

        if (MissionWidget) SavedMissionVisibility = MissionWidget->GetVisibility();
        if (OverheatWidget) SavedOverheatVisibility = OverheatWidget->GetVisibility();
        if (ItemError_Overlay) SavedItemErrorVisibility = ItemError_Overlay->GetVisibility();

        if (InteractButton) InteractButton->SetVisibility(ESlateVisibility::Collapsed);
        if (ItemBox) ItemBox->SetVisibility(ESlateVisibility::Collapsed);
        if (MissionWidget) MissionWidget->SetVisibility(ESlateVisibility::Collapsed);
        if (OverheatWidget) OverheatWidget->SetVisibility(ESlateVisibility::Collapsed);
        if (ItemError_Overlay) ItemError_Overlay->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::ShowExitHint(bool bVisible)
{
    if (Btn_ExitHint)
    {
        Btn_ExitHint->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::ShowValveOverlay(bool bVisible)
{
    if (OL_ValveImg)
    {
        OL_ValveImg->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UCPRHUDWidget::ShowGeneratorOverlay(bool bVisible)
{
    if (OL_GeneratorImg)
    {
        OL_GeneratorImg->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
