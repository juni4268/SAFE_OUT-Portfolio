#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "CPROverheatWidget.generated.h"

class UProgressBar;
class UButton;
class UWidget;
class ACPREvent_Overheat;

UCLASS()
class PROJECT_CPR_API UCPROverheatWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void ShowPanel(bool bVisible, ACPREvent_Overheat *Event = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void UpdateStats(float OilRatio, float CoolantRatio, float TempRatio);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void SimulateOilButtonPress(bool bIsPressed);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void SimulateCoolantButtonPress(bool bIsPressed);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void ShowButtonBox(bool bVisible, ACPREvent_Overheat *Event = nullptr);

    UPROPERTY(BlueprintReadOnly, Category = "Overheat UI")
    ACPREvent_Overheat *OwningEvent = nullptr;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    UWidget *OverheatPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar *OilProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar *CoolantProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar *TempProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *AddOilButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *AddCoolantButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UWidget *ButtonBox;
};
