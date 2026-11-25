#include "CPREvent_HardHat.h"
#include "CPRCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ACPREvent_HardHat::ACPREvent_HardHat()
{
	PrimaryActorTick.bCanEverTick = true;

	FallingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FallingMesh"));
	FallingMesh->SetupAttachment(RootComponent);

	FallingMesh->SetHiddenInGame(true);
	FallingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FallingMesh->SetSimulatePhysics(false);
	FallingMesh->SetNotifyRigidBodyCollision(true);

	FallingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ACPREvent_HardHat::BeginPlay()
{
	Super::BeginPlay();

	if (FallingMesh)
	{
		FallingMesh->OnComponentHit.AddDynamic(this, &ACPREvent_HardHat::OnFallingMeshHit);
	}
}

void ACPREvent_HardHat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTracking && TrackingTarget && FallingMesh)
	{
		FVector CurrentLoc = FallingMesh->GetComponentLocation();
		FVector TargetLoc = TrackingTarget->GetActorLocation();

		FallingMesh->SetWorldLocation(FVector(TargetLoc.X, TargetLoc.Y, CurrentLoc.Z));
	}
}

void ACPREvent_HardHat::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionZoneBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (bHasFallen) return;

	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		bHasFallen = true;
		TrackingTarget = Character;
		bIsTracking = true;
		StartEvent(Character);

		FVector SpawnLoc = Character->GetActorLocation() + FVector(0, 0, SpawnHeightOffset);
		FallingMesh->SetWorldLocation(SpawnLoc);

		FallingMesh->SetHiddenInGame(false);

		//FallingMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		FallingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		FallingMesh->SetSimulatePhysics(true);
	}
}

void ACPREvent_HardHat::OnFallingMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if (!bIsTracking)
	{
		return;
	}

	bIsTracking = false;

	if (ACPRCharacter* TargetCharacter = Cast<ACPRCharacter>(TrackingTarget))
	{

		if (TargetCharacter->bIsWearingHardHat)
		{
			if (HardHatHitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HardHatHitSound, Hit.ImpactPoint);
			}

			ResolveEvent();
		}
		else
		{
			if (NoHardHatHitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, NoHardHatHitSound, Hit.ImpactPoint);
			}

			TargetCharacter->EnableRagdoll();

			HandleDieSequence();
		}
	}

	FallingMesh->SetNotifyRigidBodyCollision(false);

	SetLifeSpan(DestroyDelay);
}
