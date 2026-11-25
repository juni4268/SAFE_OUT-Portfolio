#include "CPRPlayerController.h"
#include "CPRGameInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

void ACPRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->SetOverallScalabilityLevel(1);
			UserSettings->SetResolutionScaleValueEx(71.0f);
			UserSettings->ApplySettings(false);
			UserSettings->SaveSettings();
		}
	}

	if (const UCPRGameInstance* CPRGameInstance = GetGameInstance<UCPRGameInstance>())
	{
		if (CPRGameInstance->IsGameplayStartBlocked())
		{
			return;
		}
	}

	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	bShowMouseCursor = false;
}
