#include "CPREventBase.h"
#include "CPRCharacter.h"
#include "CPRDieUI.h"
#include "CPRGameInstance.h"
#include "ItemData.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"

ACPREventBase::ACPREventBase()
{

	PrimaryActorTick.bCanEverTick = true;

	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	RootComponent = InteractionZone;
	InteractionZone->SetBoxExtent(FVector(150.f, 150.f, 120.f));
	InteractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(RootComponent);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MarkerMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));

	MarkerMesh->SetCastShadow(false);

	MarkerMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
		TEXT("/Engine/BasicShapes/Plane.Plane")
	);
	if (PlaneMesh.Succeeded())
	{
		MarkerMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void ACPREventBase::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionZone)
	{
		InteractionZone->OnComponentBeginOverlap.AddDynamic(this, &ACPREventBase::OnInteractionZoneBeginOverlap);
		InteractionZone->OnComponentEndOverlap.AddDynamic(this, &ACPREventBase::OnInteractionZoneEndOverlap);
	}

	ScheduleAutoStart();

	if (MarkerMesh && MarkerMesh->GetMaterial(0))
	{
		MarkerMID = MarkerMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	UpdateMarkerTexture();

}

void ACPREventBase::SetEventStatus(ECPREventStatus NewStatus)
{
	if (CurrentStatus == NewStatus)
	{
		return;
	}

	ECPREventStatus OldStatus = CurrentStatus;
	CurrentStatus = NewStatus;

	MarkerCurrentRotation = 0.0f;
	MarkerPulseTime = 0.0f;

	OnStatusChanged(OldStatus, NewStatus);
	OnEventStatusChanged.Broadcast(NewStatus);
}

void ACPREventBase::OnStatusChanged(ECPREventStatus OldStatus, ECPREventStatus NewStatus)
{
	switch (NewStatus)
	{
		case ECPREventStatus::Active:
			if (bHasTimeLimit && TimeLimit > 0.0f && !EventTimerHandle.IsValid())
			{
				GetWorldTimerManager().SetTimer(EventTimerHandle, this, &ACPREventBase::OnTimerExpired, TimeLimit, false);
			}
			break;

		case ECPREventStatus::Resolved:
			GetWorldTimerManager().ClearTimer(EventTimerHandle);
			GetWorldTimerManager().ClearTimer(AutoStartTimerHandle);

			if (CachedCharacter && CachedCharacter->ActiveEvent == this)
			{
				CachedCharacter->ActiveEvent = nullptr;
				CachedCharacter->UpdateInteractUI();
			}
			break;

		case ECPREventStatus::Failed:
			GetWorldTimerManager().ClearTimer(EventTimerHandle);
			GetWorldTimerManager().ClearTimer(AutoStartTimerHandle);

			if (CachedCharacter && CachedCharacter->ActiveEvent == this)
			{
				CachedCharacter->ActiveEvent = nullptr;
				CachedCharacter->UpdateInteractUI();
			}

			ReceiveEventFailed();
			break;
	}

	UpdateMarkerTexture();
}

void ACPREventBase::StartEvent(ACPRCharacter* TargetCharacter)
{
	if (CurrentStatus != ECPREventStatus::Idle || !GetWorld())
	{
		return;
	}

	if (!TargetCharacter)
	{
		TargetCharacter = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	CachedCharacter = TargetCharacter;
	SetEventStatus(ECPREventStatus::Active);
	ReceiveEventStarted(TargetCharacter);
}

void ACPREventBase::ResumeAutoStartAfterGameplayStart()
{
	ScheduleAutoStart();
}

void ACPREventBase::OnTimerExpired()
{
	if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
	{
		return;
	}

	ReceiveEventTimeOut();
	SetEventStatus(ECPREventStatus::Failed);
	HandleDieSequence();
}

void ACPREventBase::ResolveEvent()
{
	if (CurrentStatus == ECPREventStatus::Resolved || CurrentStatus == ECPREventStatus::Failed)
	{
		return;
	}

	SetEventStatus(ECPREventStatus::Resolved);
	ReceiveEventResolved();

	if (bDisableCollisionOnResolve && InteractionZone)
	{
		InteractionZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (bDestroyOnResolve)
	{
		Destroy();
	}
}

void ACPREventBase::ResetResolve()
{
	if (CurrentStatus != ECPREventStatus::Resolved)
	{
		return;
	}
	SetEventStatus(ECPREventStatus::Idle);
}

void ACPREventBase::ReceiveInteractionValidated_Implementation(ACPRCharacter* TargetCharacter)
{
}

void ACPREventBase::ReceivePlayerInteraction_Implementation(ACPRCharacter* TargetCharacter)
{
	if (!TargetCharacter)
	{
		return;
	}

	if (CurrentStatus == ECPREventStatus::Idle)
	{
		TargetCharacter->ShowErrorMessage(FText::FromString(TEXT("이상 없음. 정상 가동 중")));
		return;
	}

	if (!IsInProgress())
	{
		return;
	}

	if (RequiredItemTag != FName("None"))
	{
		if (!TargetCharacter->CurrentHeldItem)
		{
			TargetCharacter->ShowErrorMessage(FText::FromString(TEXT("아이템이 없습니다!")));
			return;
		}

		if (TargetCharacter->CurrentHeldItem->ItemTag != RequiredItemTag)
		{
			TargetCharacter->ShowErrorMessage(FText::FromString(TEXT("잘못된 아이템입니다!")));
			return;
		}
	}

	ReceiveInteractionValidated(TargetCharacter);
}

void ACPREventBase::ReceivePlayerInteractionReleased_Implementation(ACPRCharacter* TargetCharacter)
{
}

void ACPREventBase::HandleDieSequence()
{
	HandleCustomDieSequence(DefaultDeathSettings);
}

void ACPREventBase::HandleCustomDieSequence(const FEventDeathSettings& DeathSettings)
{
	if (!CachedCharacter)
	{
		CachedCharacter = Cast<ACPRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
	if (!CachedCharacter)
	{
		return;
	}

	CachedCharacter->OnChokingSound(false);
	CachedCharacter->StopExtinguisher();

	float AnimDuration = 0.0f;
	if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		CachedCharacter->DisableInput(PC);
	}

	if (DeathSettings.ImpulseStrength > 0.0f)
	{
		CachedCharacter->EnableRagdoll();
		if (USkeletalMeshComponent* CharMesh = CachedCharacter->GetMesh())
		{
			FVector Direction = (CachedCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			CharMesh->AddImpulse(Direction * DeathSettings.ImpulseStrength, NAME_None, true);
		}
	}
	else if (DeathSettings.DeathAnimation)
	{
		if (UAnimInstance* AnimInstance = CachedCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathSettings.DeathAnimation);
			AnimDuration = DeathSettings.DeathAnimation->GetPlayLength();
		}
	}

	if (CachedCharacter->DieUI)
	{
		CachedCharacter->DieUI->StartDieSequence(DeathSettings.DeathResultImage, DeathSettings.DeathResultText, AnimDuration + DeathSettings.ExtraDelayAfterAnim);
	}

	if (CachedCharacter)
	{
		CachedCharacter->SetCharacterVisibility(true, false);
	}
}

void ACPREventBase::AutoStartTrigger()
{
	if (IsGameplayStartBlocked())
	{
		ScheduleAutoStart();
		return;
	}

	StartEvent(nullptr);
}

void ACPREventBase::ScheduleAutoStart()
{
	if (!bUseAutoStart || AutoStartDelay <= 0.0f || CurrentStatus != ECPREventStatus::Idle || !GetWorld())
	{
		return;
	}

	if (IsGameplayStartBlocked())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoStartTimerHandle);
	GetWorldTimerManager().SetTimer(AutoStartTimerHandle, this, &ACPREventBase::AutoStartTrigger, AutoStartDelay, false);
}

bool ACPREventBase::IsGameplayStartBlocked() const
{
	if (!GetWorld())
	{
		return false;
	}

	if (const UCPRGameInstance* CPRGameInstance = GetWorld()->GetGameInstance<UCPRGameInstance>())
	{
		return CPRGameInstance->IsGameplayStartBlocked();
	}

	return false;
}

void ACPREventBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickMarker(DeltaTime);
}

void ACPREventBase::UpdateMarkerTexture()
{
	if (!MarkerMesh)
	{
		return;
	}

	UTexture2D* TargetTexture = GetOverrideMarkerTexture();
	if (!TargetTexture)
	{
		switch (CurrentStatus)
		{
			case ECPREventStatus::Idle:
				TargetTexture = MarkerTexture_Idle;
				break;
			case ECPREventStatus::Active:
			case ECPREventStatus::Interacting:
				TargetTexture = MarkerTexture_Active;
				break;
			case ECPREventStatus::Resolved:
				TargetTexture = MarkerTexture_Resolved;
				break;
			case ECPREventStatus::Failed:
				TargetTexture = MarkerTexture_Failed;
				break;
		}
	}

	if (!TargetTexture)
	{
		MarkerMesh->SetVisibility(false);
		return;
	}

	MarkerMesh->SetVisibility(true);
	if (MarkerMID)
	{
		MarkerMID->SetTextureParameterValue(FName("MarkerTexture"), TargetTexture);
	}
}

void ACPREventBase::TickMarker(float DeltaTime)
{
	if (!MarkerMesh || !MarkerMesh->IsVisible())
	{
		return;
	}

	APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CamMgr)
	{
		FVector CamLoc    = CamMgr->GetCameraLocation();
		FVector MarkerLoc = MarkerMesh->GetComponentLocation();
		FVector ToCam     = (CamLoc - MarkerLoc).GetSafeNormal();

		FRotator FaceRot  = UKismetMathLibrary::MakeRotFromZY(ToCam, -FVector::UpVector);
		MarkerMesh->SetWorldRotation(FaceRot);

		FMarkerAnimSettings CurrentAnim;
		if (FMarkerAnimSettings* OverrideAnim = GetOverrideMarkerAnim())
		{
			CurrentAnim = *OverrideAnim;
		}
		else
		{
			switch (CurrentStatus)
			{
				case ECPREventStatus::Idle: CurrentAnim = Anim_Idle; break;
				case ECPREventStatus::Active:
				case ECPREventStatus::Interacting: CurrentAnim = Anim_Active; break;
				case ECPREventStatus::Resolved: CurrentAnim = Anim_Resolved; break;
				case ECPREventStatus::Failed: CurrentAnim = Anim_Failed; break;
			}
		}

		if (CurrentAnim.RotationSpeed != 0.0f)
		{
			MarkerCurrentRotation += CurrentAnim.RotationSpeed * DeltaTime;
			MarkerCurrentRotation = FMath::Fmod(MarkerCurrentRotation, 360.0f);
		}
		else
		{

			MarkerCurrentRotation = 0.0f;
		}

		if (MarkerCurrentRotation != 0.0f)
		{

			MarkerMesh->AddLocalRotation(FRotator(0.0f, MarkerCurrentRotation, 0.0f));
		}

		if (CurrentAnim.PulseAmplitude > 0.0f)
		{
			MarkerPulseTime += DeltaTime * CurrentAnim.PulseSpeed;

			float PulseScale = MarkerBaseScale + (FMath::Sin(MarkerPulseTime * PI * 2.0f) * CurrentAnim.PulseAmplitude);
			MarkerMesh->SetWorldScale3D(FVector(PulseScale));
		}
		else
		{
			MarkerMesh->SetWorldScale3D(FVector(MarkerBaseScale));
		}
	}
}

void ACPREventBase::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		bIsPlayerInRange = true;
		if (bShowInteractionUI)
		{
			Character->ActiveEvent = this;
			Character->UpdateInteractUI();
		}
	}
}

void ACPREventBase::OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACPRCharacter* Character = Cast<ACPRCharacter>(OtherActor))
	{
		bIsPlayerInRange = false;
		if (Character->ActiveEvent == this)
		{
			Character->ActiveEvent = nullptr;
			Character->UpdateInteractUI();
		}
	}
}
