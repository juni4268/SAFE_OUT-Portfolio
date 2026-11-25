#include "CPREvent_FireLithium.h"
#include "CPRCharacter.h"
#include "CPRHUDWidget.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACPREvent_FireLithium::ACPREvent_FireLithium()
{
	PrimaryActorTick.bCanEverTick = true;

	bHasTimeLimit = false;
	bUseAutoStart = true;
	AutoStartDelay = 1.0f;

	RequiredItemTag = FName("Lithium");

	BurnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BurnZone"));
	BurnZone->SetupAttachment(RootComponent);
	BurnZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	BurnZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BurnZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	BurnZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	FireAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComp"));
	FireAudioComp->SetupAttachment(RootComponent);
	FireAudioComp->bAutoActivate = false;
}

void ACPREvent_FireLithium::BeginPlay()
{
	Super::BeginPlay();

	if (BurnZone)
	{
		BurnZone->OnComponentBeginOverlap.AddDynamic(this, &ACPREvent_FireLithium::OnBurnZoneBeginOverlap);
	}

	if (FireAudioComp && FireSound)
	{
		FireAudioComp->SetSound(FireSound);
	}
}

void ACPREvent_FireLithium::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{
	if (NewStatus == ECPREventStatus::Failed || NewStatus == ECPREventStatus::Resolved)
	{
		bIsOnFire = false;
		if (FireAudioComp && FireAudioComp->IsPlaying())
		{
			FireAudioComp->Stop();
		}
	}

	Super::OnStatusChanged(OldStatus, NewStatus);
}

void ACPREvent_FireLithium::ReceivePlayerInteraction_Implementation(ACPRCharacter* TargetCharacter)
{
	if (!TargetCharacter)
	{
		return;
	}

	if (bIsOnFire)
	{
		TargetCharacter->ShowErrorMessage(FText::FromString(TEXT("D급 금속화재 소화기가 필요합니다!")));
		return;
	}

	Super::ReceivePlayerInteraction_Implementation(TargetCharacter);
}

void ACPREvent_FireLithium::ReceiveInteractionValidated_Implementation(ACPRCharacter* TargetCharacter)
{
	if (!TargetCharacter)
	{
		return;
	}

	TargetCharacter->CurrentHeldItem = nullptr;
	if (TargetCharacter->HUDWidget)
	{
		TargetCharacter->HUDWidget->UpdateAcquisitionInfo(nullptr, FText::GetEmpty());
	}

	LithiumCurrent += LithiumPerInteraction;
	ReceiveLithiumChanged(LithiumCurrent, LithiumMax);

	UE_LOG(LogTemp, Log, TEXT("FireLithium: Lithium deposited. Current: %.1f / Max: %.1f"), LithiumCurrent, LithiumMax);

	if (LithiumCurrent >= LithiumMax)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(FireDelayTimerHandle, this, &ACPREvent_FireLithium::TriggerFire, FireTriggerDelay, false);
		}
	}
}

void ACPREvent_FireLithium::TriggerFire()
{
	if (bIsOnFire)
	{
		return;
	}

	bIsOnFire = true;
	FireLevel = 1.0f;

	UE_LOG(LogTemp, Warning, TEXT("FireLithium: Fire triggered!"));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EventTimerHandle, this,
			&ACPREvent_FireLithium::OnTimerExpired,
			FireTimeLimit, false);
	}

	if (FireAudioComp && !FireAudioComp->IsPlaying() && FireAudioComp->Sound)
	{
		FireAudioComp->Play();
	}

	ReceiveFireStarted();
	ReceiveFireLevelChanged(FireLevel);

	UpdateMarkerTexture();

	if (BurnZone)
	{
		TArray<AActor*> OverlappingActors;
		BurnZone->GetOverlappingActors(OverlappingActors, ACPRCharacter::StaticClass());

		if (OverlappingActors.Num() > 0)
		{
			ReceiveLithiumBurnedDeath();
			SetEventStatus(ECPREventStatus::Failed);
			HandleCustomDieSequence(BurnedDeathSettings);
		}
	}
}

void ACPREvent_FireLithium::OnTimerExpired()
{
	if (!bIsOnFire)
	{
		return;
	}
	if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
	{
		return;
	}

	ReceiveEventTimeOut();
	SetEventStatus(ECPREventStatus::Failed);
	HandleCustomDieSequence(TimeOutDeathSettings);
}

bool ACPREvent_FireLithium::ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag)
{

	if (ExtinguisherTag != AcceptedExtinguisherTag)
	{
		return false;
	}

	if (!bIsOnFire || CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
	{
		return false;
	}

	FireLevel -= Amount;
	FireLevel = FMath::Max(FireLevel, 0.f);
	ReceiveFireLevelChanged(FireLevel);

	if (FireLevel <= 0.f)
	{
		if (FireAudioComp && FireAudioComp->IsPlaying())
		{
			FireAudioComp->Stop();
		}
		ResolveEvent();
	}

	return true;
}

void ACPREvent_FireLithium::OnInteractionZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	bIsPlayerInRange = true;
	Character->ActiveEvent = this;
	CachedCharacter = Character;

	if (Character->HUDWidget && bShowInteractionUI)
	{
		Character->HUDWidget->UpdateInteractVisibility(true);
	}
}

void ACPREvent_FireLithium::OnInteractionZoneEndOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	bIsPlayerInRange = false;

	if (Character->HUDWidget)
	{
		Character->HUDWidget->UpdateInteractVisibility(false);
	}

	if (Character->ActiveEvent == this)
	{
		Character->ActiveEvent = nullptr;
	}
}

void ACPREvent_FireLithium::OnBurnZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsOnFire || CurrentStatus == ECPREventStatus::Failed || CurrentStatus == ECPREventStatus::Resolved)
	{
		return;
	}

	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		ReceiveLithiumBurnedDeath();
		SetEventStatus(ECPREventStatus::Failed);
		HandleCustomDieSequence(BurnedDeathSettings);
	}
}

UTexture2D* ACPREvent_FireLithium::GetOverrideMarkerTexture() const
{
	if (bIsOnFire)
	{
		return MarkerTexture_OnFire;
	}
	return nullptr;
}

FMarkerAnimSettings* ACPREvent_FireLithium::GetOverrideMarkerAnim()
{
	if (bIsOnFire)
	{
		return &Anim_OnFire;
	}
	return nullptr;
}
