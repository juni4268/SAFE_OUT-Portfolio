#include "CPRDieUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UCPRDieUI::NativeDestruct()
{
	Super::NativeDestruct();
	GetWorld()->GetTimerManager().ClearTimer(DieTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RestartTimerHandle);
}

void UCPRDieUI::StartDieSequence(UTexture2D* DieImage, FText DieText, float Delay)
{
	PendingImage = DieImage;
	PendingText = DieText;

	if (Delay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DieTimerHandle, this, &UCPRDieUI::ExecuteDieEffect, Delay, false);
	}
	else
	{
		ExecuteDieEffect();
	}
}

void UCPRDieUI::ExecuteDieEffect()
{
	SetVisibility(ESlateVisibility::Visible);

	if (Die_Image && PendingImage)
	{
		Die_Image->SetBrushFromTexture(PendingImage);
	}

	if (Die_Text && !PendingText.IsEmpty())
	{
		Die_Text->SetText(PendingText);
	}

	OnDieSequenceFinished(PendingImage);

	if (RestartDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(RestartTimerHandle, this, &UCPRDieUI::RestartLevel, RestartDelay, false);
	}
}

void UCPRDieUI::RestartLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}
