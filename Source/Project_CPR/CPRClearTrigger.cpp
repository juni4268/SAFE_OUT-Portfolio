#include "CPRClearTrigger.h"

#include "CPRCharacter.h"
#include "CPREvent_Overheat.h"
#include "CPRHUDWidget.h"
#include "CPRMissionWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

ACPRClearTrigger::ACPRClearTrigger()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	SetRootComponent(TriggerZone);
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(RootComponent);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	MarkerMesh->SetCastShadow(false);
	MarkerMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		MarkerMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void ACPRClearTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACPRClearTrigger::OnTriggerBeginOverlap);

	if (MarkerMesh && MarkerMesh->GetMaterial(0))
	{
		MarkerMID = MarkerMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	TryBindMissionWidget();
}

void ACPRClearTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bConditionsMet && !bCleared)
	{
		TickMarker(DeltaTime);
	}

	if (!bMissionWidgetBound && !bConditionsMet)
	{
		TryBindMissionWidget();
	}
}

void ACPRClearTrigger::OnMissionAllCompleted()
{
	if (bConditionsMet)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[CPRClearTrigger] All missions completed signal received"));
	HandleConditionsMet();
}

bool ACPRClearTrigger::TryBindMissionWidget()
{
	ACPRCharacter* Player = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Player || !Player->HUDWidget)
	{
		return false;
	}

	UCPRMissionWidget* MissionWidget = Player->HUDWidget->GetMissionWidget();
	if (!MissionWidget)
	{
		return false;
	}

	MissionWidget->OnAllMissionsCompleted.AddUniqueDynamic(this, &ACPRClearTrigger::OnMissionAllCompleted);
	bMissionWidgetBound = true;

	UE_LOG(LogTemp, Log, TEXT("[CPRClearTrigger] MissionWidget delegate bound"));

	if (MissionWidget->IsAllMissionsCompleted())
	{
		OnMissionAllCompleted();
	}

	return true;
}

void ACPRClearTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bCleared)
	{
		return;
	}

	if (!Cast<ACPRCharacter>(OtherActor))
	{
		return;
	}

	if (bConditionsMet)
	{
		bCleared = true;
		UE_LOG(LogTemp, Log, TEXT("[CPRClearTrigger] Player entered clear trigger"));
		SuspendOverheatsForClear();
		StopTaggedWorldAudio();
		if (bLockOnClear)
		{
			LockForClear();
		}
		ReceiveLevelCleared();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPRClearTrigger] Clear trigger entered before missions completed"));
	}
}

void ACPRClearTrigger::HandleConditionsMet()
{
	bConditionsMet = true;
	UE_LOG(LogTemp, Log, TEXT("[CPRClearTrigger] Clear conditions met"));

	UpdateMarkerTexture();

	if (ACPRCharacter* Player = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		if (Player->HUDWidget)
		{
			Player->HUDWidget->ShowClearText();
		}
	}

	OnClearConditionsMet();
}

void ACPRClearTrigger::LockForClear()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	if (ACPRCharacter* Player = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		Player->StopExtinguisher();
		Player->ActiveEvent = nullptr;
		Player->UpdateInteractUI();
	}
}

void ACPRClearTrigger::SuspendOverheatsForClear()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ACPREvent_Overheat> It(World); It; ++It)
	{
		It->SuspendForClear();
	}
}

void ACPRClearTrigger::StopTaggedWorldAudio()
{
	UWorld* World = GetWorld();
	if (!World || AudioStopTag.IsNone())
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		const bool bStopAllAudioOnActor = It->ActorHasTag(AudioStopTag);

		TArray<UAudioComponent*> AudioComponents;
		It->GetComponents(AudioComponents);

		for (UAudioComponent* AudioComponent : AudioComponents)
		{
			if (!AudioComponent || !AudioComponent->IsPlaying())
			{
				continue;
			}

			if (bStopAllAudioOnActor || AudioComponent->ComponentHasTag(AudioStopTag))
			{
				AudioComponent->Stop();
			}
		}
	}
}

void ACPRClearTrigger::UpdateMarkerTexture()
{
	if (!MarkerMesh)
	{
		return;
	}

	if (!MarkerTexture)
	{
		MarkerMesh->SetVisibility(false);
		return;
	}

	MarkerMesh->SetVisibility(true);

	if (MarkerMID)
	{
		MarkerMID->SetTextureParameterValue(FName("MarkerTexture"), MarkerTexture);
	}
}

void ACPRClearTrigger::TickMarker(float DeltaTime)
{
	if (!MarkerMesh || !MarkerMesh->IsVisible())
	{
		return;
	}

	APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CamMgr)
	{
		FVector CamLoc = CamMgr->GetCameraLocation();
		FVector MarkerLoc = MarkerMesh->GetComponentLocation();
		FVector ToCam = (CamLoc - MarkerLoc).GetSafeNormal();

		FRotator FaceRot = UKismetMathLibrary::MakeRotFromZY(ToCam, -FVector::UpVector);
		MarkerMesh->SetWorldRotation(FaceRot);
	}

	MarkerPulseTime += DeltaTime * PulseSpeed;
	const float PulseFactor = 1.0f + PulseAmplitude * FMath::Sin(MarkerPulseTime * 2.0f * PI);
	const float FinalScale = MarkerBaseScale * PulseFactor;
	MarkerMesh->SetWorldScale3D(FVector(FinalScale));
}
