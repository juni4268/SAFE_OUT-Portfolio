#include "CPRGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "MoviePlayer.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UCPRGameInstance::Init()
{
	Super::Init();

	if (LoadingText.IsEmpty())
	{
		LoadingText = FText::FromString(TEXT("Loading..."));
	}

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UCPRGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UCPRGameInstance::EndLoadingScreen);
}

void UCPRGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Shutdown();
}

void UCPRGameInstance::RequestPostLoadCutscene(FName CutsceneTag)
{
	PendingPostLoadCutsceneTag = CutsceneTag;
	bGameplayStartBlocked = !CutsceneTag.IsNone();
}

bool UCPRGameInstance::ConsumePendingPostLoadCutscene(FName& OutCutsceneTag)
{
	if (PendingPostLoadCutsceneTag.IsNone())
	{
		OutCutsceneTag = NAME_None;
		return false;
	}

	OutCutsceneTag = PendingPostLoadCutsceneTag;
	PendingPostLoadCutsceneTag = NAME_None;
	return true;
}

void UCPRGameInstance::NotifyPostLoadCutsceneFinished()
{
	bGameplayStartBlocked = false;
}

bool UCPRGameInstance::HasPostLoadCutsceneRequest() const
{
	return !PendingPostLoadCutsceneTag.IsNone();
}

bool UCPRGameInstance::IsGameplayStartBlocked() const
{
	return bGameplayStartBlocked;
}

void UCPRGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	if (GIsEditor && !bShowLoadingScreenInEditor)
	{
		return;
	}

	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.MinimumLoadingScreenDisplayTime = MinimumLoadingScreenDisplayTime;

	if (LoadingScreenWidgetClass)
	{
		if (UUserWidget* LoadingWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass))
		{
			LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget();
		}
	}

	if (!LoadingScreen.WidgetLoadingScreen.IsValid())
	{
		LoadingScreen.WidgetLoadingScreen = CreateDefaultLoadingScreenWidget();
	}

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void UCPRGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
}

TSharedRef<SWidget> UCPRGameInstance::CreateDefaultLoadingScreenWidget() const
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("BlackBrush"))
		.Padding(0.0f)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LoadingText)
				.ColorAndOpacity(FLinearColor::White)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
			]
		];
}
