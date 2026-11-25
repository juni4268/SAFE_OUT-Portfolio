#include "CPROverheatWidget.h"
#include "CPREvent_Overheat.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"

void UCPROverheatWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ShowPanel(false);
    ShowButtonBox(false);
}

void UCPROverheatWidget::ShowPanel(bool bVisible, ACPREvent_Overheat *Event)
{

    OwningEvent = bVisible ? Event : nullptr;

    if (OverheatPanel)
    {
        OverheatPanel->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }

    if (!bVisible)
    {
        ShowButtonBox(false);
    }
}

void UCPROverheatWidget::UpdateStats(float OilRatio, float CoolantRatio, float TempRatio)
{

    if (OilProgressBar)
    {
        OilProgressBar->SetPercent(OilRatio);
        OilProgressBar->SetFillColorAndOpacity(FLinearColor(FColor(74, 38, 16)));
    }

    if (CoolantProgressBar)
    {
        CoolantProgressBar->SetPercent(CoolantRatio);
        CoolantProgressBar->SetFillColorAndOpacity(FLinearColor(FColor(21, 96, 189)));
    }

    if (TempProgressBar)
    {
        TempProgressBar->SetPercent(TempRatio);
        const FLinearColor TempLow(1.0f, 0.9f, 0.0f);
        const FLinearColor TempHigh(1.0f, 0.0f, 0.0f);
        TempProgressBar->SetFillColorAndOpacity(FLinearColor::LerpUsingHSV(TempLow, TempHigh, TempRatio));
    }
}

void UCPROverheatWidget::SimulateOilButtonPress(bool bIsPressed)
{
    if (AddOilButton)
    {
        AddOilButton->SetBackgroundColor(bIsPressed ? FLinearColor::Gray : FLinearColor::White);
    }
}

void UCPROverheatWidget::SimulateCoolantButtonPress(bool bIsPressed)
{
    if (AddCoolantButton)
    {
        AddCoolantButton->SetBackgroundColor(bIsPressed ? FLinearColor::Gray : FLinearColor::White);
    }
}

void UCPROverheatWidget::ShowButtonBox(bool bVisible, ACPREvent_Overheat* Event)
{
    if (bVisible && Event)
    {
        OwningEvent = Event;
    }

    if (ButtonBox)
    {
        ButtonBox->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
