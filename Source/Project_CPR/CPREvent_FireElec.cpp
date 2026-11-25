#include "CPREvent_FireElec.h"
#include "CPRCharacter.h"
#include "CPREvent_Generator.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACPREvent_FireElec::ACPREvent_FireElec()
{
	PrimaryActorTick.bCanEverTick = true;

	bHasTimeLimit = false;
	bUseAutoStart = true;
	AutoStartDelay = 5.0f;

	EventMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EventMesh"));
	EventMesh->SetupAttachment(RootComponent);

	ElecBurnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ElecBurnZone"));
	ElecBurnZone->SetupAttachment(EventMesh);
	ElecBurnZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	ElecBurnZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ElecBurnZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ElecBurnZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	NormalBurnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("NormalBurnZone"));
	NormalBurnZone->SetupAttachment(EventMesh);
	NormalBurnZone->SetBoxExtent(FVector(150.f, 150.f, 150.f));
	NormalBurnZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NormalBurnZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	NormalBurnZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	FireAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComp"));
	FireAudioComp->SetupAttachment(EventMesh);
	FireAudioComp->bAutoActivate = false;
}

void ACPREvent_FireElec::BeginPlay()
{
	Super::BeginPlay();

	if (ElecBurnZone)
	{
		ElecBurnZone->OnComponentBeginOverlap.AddDynamic(this, &ACPREvent_FireElec::OnElecBurnZoneBeginOverlap);
	}

	if (NormalBurnZone)
	{
		NormalBurnZone->OnComponentBeginOverlap.AddDynamic(this, &ACPREvent_FireElec::OnNormalBurnZoneBeginOverlap);
	}

	if (FireAudioComp && FireSound)
	{
		FireAudioComp->SetSound(FireSound);
	}
}

void ACPREvent_FireElec::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{
	Super::OnStatusChanged(OldStatus, NewStatus);

	if (NewStatus == ECPREventStatus::Active)
	{

		TriggerFire();

		if (LinkedGenerator)
		{
			LinkedGenerator->StartEvent(nullptr);
		}
	}

	if (NewStatus == ECPREventStatus::Failed || NewStatus == ECPREventStatus::Resolved)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);

		if (FireAudioComp && FireAudioComp->IsPlaying())
		{
			FireAudioComp->Stop();
		}
	}
}

void ACPREvent_FireElec::TriggerFire()
{
	if (bIsOnFire)
	{
		return;
	}

	bIsOnFire = true;
	FireLevel = 1.0f;

	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ACPREvent_FireElec::OnTimerExpired,
		FireTimeLimit,
		false
	);

	if (FireAudioComp && !FireAudioComp->IsPlaying() && FireAudioComp->Sound)
	{
		FireAudioComp->Play();
	}

	ReceiveFireStarted();
	ReceiveFireLevelChanged(FireLevel);

	TArray<AActor*> OverlappingActors;

	if (ElecBurnZone && bElecBurnZoneActive)
	{
		ElecBurnZone->GetOverlappingActors(OverlappingActors, ACPRCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
            ReceiveElecBurnedDeath();
			SetEventStatus(ECPREventStatus::Failed);
			HandleCustomDieSequence(ElecBurnedDeathSettings);
			return;
		}
	}

	if (NormalBurnZone)
	{
		NormalBurnZone->GetOverlappingActors(OverlappingActors, ACPRCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
			SetEventStatus(ECPREventStatus::Failed);
			HandleCustomDieSequence(NormalBurnedDeathSettings);
		}
	}
}

void ACPREvent_FireElec::OnTimerExpired()
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

void ACPREvent_FireElec::SetElecBurnZoneActive(bool bActive)
{
	bElecBurnZoneActive = bActive;

	if (ElecBurnZone)
	{

		ElecBurnZone->SetCollisionEnabled(
			bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
		);
	}
}

bool ACPREvent_FireElec::ApplyExtinguisher_Implementation(float Amount, FName ExtinguisherTag)
{

	if (ExtinguisherTag != AcceptedExtinguisherTag)
	{
		return false;
	}

	if (!bIsOnFire)
	{
		return false;
	}

	if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
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
		bIsOnFire = false;
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		ResolveEvent();
	}

	return true;
}

void ACPREvent_FireElec::OnElecBurnZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{

	if (!bElecBurnZoneActive || !bIsOnFire
		|| CurrentStatus == ECPREventStatus::Failed
		|| CurrentStatus == ECPREventStatus::Resolved)
	{
		return;
	}

	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
        ReceiveElecBurnedDeath();
		SetEventStatus(ECPREventStatus::Failed);
		HandleCustomDieSequence(ElecBurnedDeathSettings);
	}
}

void ACPREvent_FireElec::OnNormalBurnZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{

	if (!bIsOnFire
		|| CurrentStatus == ECPREventStatus::Failed
		|| CurrentStatus == ECPREventStatus::Resolved)
	{
		return;
	}

	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		SetEventStatus(ECPREventStatus::Failed);
		HandleCustomDieSequence(NormalBurnedDeathSettings);
	}
}
