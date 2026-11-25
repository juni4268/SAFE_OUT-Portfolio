#pragma once

#include "Blueprint/UserWidget.h"
#include "CPREventBase.h"
#include "CoreMinimal.h"
#include "CPRHUDWidget.generated.h"

class UImage;
class UTextBlock;
class UOverlay;
class UButton;
class UWidget;
class UCPROverheatWidget;
class UCPRPauseWidget;
class ACPREvent_Overheat;

UCLASS()
class PROJECT_CPR_API UCPRHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UImage *Item_Image;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Item_Text;

    UPROPERTY(meta = (BindWidgetOptional))
    class UWidget *ItemBox;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay *ItemError_Overlay;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock *ItemError_Text;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *InteractButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay *OL_Start;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay *OL_Clear;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay *OL_ValveImg;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay *OL_GeneratorImg;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton *Btn_ExitHint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    float StartMessageDuration = 3.0f;

    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Interact UI")
    void UpdateInteractVisibility(bool bIsVisible);

    UFUNCTION(BlueprintCallable, Category = "Interact UI")
    void SimulateInteractButtonPress(bool bIsPressed);

    UFUNCTION(BlueprintCallable, Category = "Item UI")
    void UpdateAcquisitionInfo(UTexture2D *NewIcon, FText NewName);

    UFUNCTION(BlueprintCallable, Category = "Item UI")
    void ShowErrorMessage(FText Message, float DisplayTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void ShowOverheatPanel(bool bVisible, ACPREvent_Overheat *OwnerEvent = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void UpdateOverheatStats(float OilRatio, float CoolantRatio, float TempRatio);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void SimulateOilButtonPress(bool bIsPressed);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void SimulateCoolantButtonPress(bool bIsPressed);

    UFUNCTION(BlueprintCallable, Category = "Overheat UI")
    void ShowOverheatButtonBox(bool bVisible, ACPREvent_Overheat *Event = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Clear UI")
    void ShowClearText();

    UFUNCTION(BlueprintPure, Category = "Mission")
    UCPRMissionWidget* GetMissionWidget() const;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetGameplayHUDVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowExitHint(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowValveOverlay(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowGeneratorOverlay(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Pause UI")
    void ShowPauseMenu(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Pause UI")
    void TogglePauseMenu();

protected:
    UFUNCTION()
    void OnExitHintClicked();
    UFUNCTION(BlueprintImplementableEvent, Category = "Item UI")
    void OnItemAcquired();

    UFUNCTION(BlueprintImplementableEvent, Category = "Clear UI")
    void OnClearTextShown();

    UPROPERTY(meta = (BindWidgetOptional))
    UCPROverheatWidget *OverheatWidget;

    UPROPERTY(meta = (BindWidgetOptional))
    UCPRPauseWidget *PauseWidget;

    UPROPERTY(meta = (BindWidgetOptional))
    class UCPRMissionWidget *MissionWidget;

private:
    void HideItemDropError();
    FTimerHandle ItemErrorOverlayTimerHandle;

    ESlateVisibility SavedMissionVisibility = ESlateVisibility::Visible;
    ESlateVisibility SavedOverheatVisibility = ESlateVisibility::Collapsed;
    ESlateVisibility SavedItemErrorVisibility = ESlateVisibility::Collapsed;

    void HideStartOverlay();
    FTimerHandle StartOverlayTimerHandle;
};
