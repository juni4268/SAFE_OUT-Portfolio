#include "CPRGameFlowManager.h"

#include "CPREventBase.h"
#include "CPRGameInstance.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ACPRGameFlowManager::ACPRGameFlowManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPRGameFlowManager::BeginPlay()
{
	Super::BeginPlay();

	const UCPRGameInstance* CPRGameInstance = GetGameInstance<UCPRGameInstance>();
	if (!CPRGameInstance || !CPRGameInstance->IsGameplayStartBlocked())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ACPRGameFlowManager::StartGameplay);
	}
}

bool ACPRGameFlowManager::TryConsumeIntroCutsceneRequest(FName& OutCutsceneTag)
{
	if (UCPRGameInstance* CPRGameInstance = GetGameInstance<UCPRGameInstance>())
	{
		return CPRGameInstance->ConsumePendingPostLoadCutscene(OutCutsceneTag);
	}

	OutCutsceneTag = NAME_None;
	return false;
}

void ACPRGameFlowManager::NotifyIntroCutsceneFinished()
{
	StartGameplay();
}

void ACPRGameFlowManager::StartGameplay()
{
	if (bGameplayStarted)
	{
		return;
	}

	bGameplayStarted = true;

	if (UCPRGameInstance* CPRGameInstance = GetGameInstance<UCPRGameInstance>())
	{
		CPRGameInstance->NotifyPostLoadCutsceneFinished();
	}

	ResumeEventAutoStarts();
	OnGameplayStarted.Broadcast();
}

void ACPRGameFlowManager::ResumeEventAutoStarts()
{
	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<ACPREventBase> It(GetWorld()); It; ++It)
	{
		It->ResumeAutoStartAfterGameplayStart();
	}
}
