#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "CPRPauseWidget.generated.h"

class UWidget;
class UButton;

UCLASS()
class PROJECT_CPR_API UCPRPauseWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Pause UI")
    void ShowPauseMenu(bool bVisible);

    bool IsPauseMenuVisible() const { return bIsPauseMenuVisible; }

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    UWidget *PauseMenu;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *Btn_Continue;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *Btn_Restart;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *Btn_Quit;

private:

    UFUNCTION()
    void OnContinueClicked();

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnQuitClicked();

private:
    bool bIsPauseMenuVisible = false;
};
