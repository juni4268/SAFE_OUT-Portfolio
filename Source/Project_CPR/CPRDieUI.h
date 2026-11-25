#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "CPRDieUI.generated.h"

class UTexture2D;
class UImage;
class UTextBlock;

UCLASS()
class PROJECT_CPR_API UCPRDieUI : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeDestruct() override;

    void StartDieSequence(UTexture2D *DieImage, FText DieText, float Delay);

protected:
    UPROPERTY(meta = (BindWidget))
    UImage *Die_Image;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Die_Text;

    UFUNCTION(BlueprintImplementableEvent, Category = "DieUI")
    void OnDieSequenceFinished(UTexture2D *FinalImage);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DieUI")
    float RestartDelay = 3.0f;

private:
    FTimerHandle DieTimerHandle;
    FTimerHandle RestartTimerHandle;

    UPROPERTY()
    UTexture2D *PendingImage;

    FText PendingText;

    void ExecuteDieEffect();
    void RestartLevel();
};
