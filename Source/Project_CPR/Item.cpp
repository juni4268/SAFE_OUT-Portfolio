#include "Item.h"
#include "ItemData.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "TimerManager.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetBoxExtent(FVector(30.0f, 30.0f, 30.0f));
	CollisionBox->SetCollisionProfileName(TEXT("PhysicsActor"));
	CollisionBox->SetSimulatePhysics(false);
	CollisionBox->SetEnableGravity(false);
	CollisionBox->SetGenerateOverlapEvents(false);
	CollisionBox->SetLinearDamping(1.0f);
	CollisionBox->SetAngularDamping(10.0f);

	// 2. Static Mesh
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightMesh"));
	HighlightMesh->SetupAttachment(ItemMesh);
	HighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightMesh->SetCastShadow(false);
	HighlightMesh->SetVisibility(false);

	// 3. Skeletal Mesh
	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	ItemSkeletalMesh->SetupAttachment(RootComponent);
	ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HighlightSkeletalMesh"));
	HighlightSkeletalMesh->SetupAttachment(ItemSkeletalMesh);
	HighlightSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightSkeletalMesh->SetCastShadow(false);
	HighlightSkeletalMesh->SetVisibility(false);

	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidget->SetupAttachment(RootComponent);
	NameWidget->SetWidgetSpace(EWidgetSpace::World);
	NameWidget->SetDrawAtDesiredSize(true);
	NameWidget->SetRelativeScale3D(FVector(0.5f));
	NameWidget->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	EnsureNameWidgetVisible();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateAppearance();
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (NameWidget)
	{
		APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
		if (CamMgr)
		{
			FVector CamLoc = CamMgr->GetCameraLocation();
			FVector WidgetLoc = NameWidget->GetComponentLocation();

			float DistToCam = FVector::Distance(CamLoc, WidgetLoc);

			if (DistToCam <= NameVisibleDistance)
			{
				EnsureNameWidgetVisible();

				FVector ToCam = (CamLoc - WidgetLoc).GetSafeNormal();

				FRotator FaceRot = UKismetMathLibrary::MakeRotFromXZ(ToCam, FVector::UpVector);
				NameWidget->SetWorldRotation(FaceRot);
			}
			else
			{
				if (NameWidget->IsVisible())
				{
					NameWidget->SetVisibility(false);
				}
			}
		}
	}
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("BeginPlay_BeforeUpdateAppearance"));
	UpdateAppearance();
	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("BeginPlay_AfterUpdateAppearance"));

	if (CollisionBox)
	{

		CollisionBox->SetSimulatePhysics(false);
		CollisionBox->SetEnableGravity(false);
	}

	if (SpawnLiftHeight > 0.0f)
	{
		SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, SpawnLiftHeight), false);
	}

	FTimerDelegate SettleDelegate;
	SettleDelegate.BindLambda([this]()
	{
		if (!IsValid(this))
		{
			return;
		}

		if (bSnapToGroundOnBeginPlay)
		{
			SnapToGround();
		}

		if (CollisionBox)
		{

			CollisionBox->SetGenerateOverlapEvents(true);
			CollisionBox->UpdateOverlaps();
		}
	});

	if (PostSpawnSettleDelay <= 0.0f)
	{
		SettleDelegate.ExecuteIfBound();
	}
	else
	{
		FTimerHandle TmpHandle;
		GetWorldTimerManager().SetTimer(TmpHandle, SettleDelegate, PostSpawnSettleDelay, false);
	}
}

void AItem::InitializeItem(UItemData* NewData)
{
	if (!NewData) return;

	ItemData = NewData;
	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("InitializeItem_BeforeUpdateAppearance"));
	UpdateAppearance();
	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("InitializeItem_AfterUpdateAppearance"));

	if (CollisionBox)
	{
		CollisionBox->SetSimulatePhysics(false);
		CollisionBox->SetEnableGravity(false);
	}

	if (bSnapToGroundOnBeginPlay)
	{
		SnapToGround();
	}
}

void AItem::UpdateAppearance()
{
	if (!ItemData)
	{
		LogNameWidgetState(TEXT("UpdateAppearance_NoItemData"));
		return;
	}

	if (!AppliedItemTag.IsNone())
	{
		Tags.Remove(AppliedItemTag);
		AppliedItemTag = NAME_None;
	}

	if (!ItemData->ItemTag.IsNone())
	{
		Tags.AddUnique(ItemData->ItemTag);
		AppliedItemTag = ItemData->ItemTag;

	}

	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(ItemData->BoxExtent);
	}

	if (ItemData->PickupMesh)
	{
		if (ItemMesh)
		{
			ItemMesh->SetStaticMesh(ItemData->PickupMesh);
			ItemMesh->SetRelativeLocation(ItemData->MeshRelativeLocation);
			ItemMesh->SetRelativeScale3D(ItemData->MeshScale);
			ItemMesh->SetVisibility(true);

			if (HighlightMesh)
			{
				HighlightMesh->SetStaticMesh(ItemData->PickupMesh);
				HighlightMesh->SetRelativeScale3D(FVector(1.0f));
			}
		}
		if (ItemSkeletalMesh) ItemSkeletalMesh->SetVisibility(false);
	}
	else if (ItemData->PickupSkeletalMesh)
	{
		if (ItemSkeletalMesh)
		{
			ItemSkeletalMesh->SetSkeletalMeshAsset(ItemData->PickupSkeletalMesh);
			ItemSkeletalMesh->SetRelativeLocation(ItemData->MeshRelativeLocation);
			ItemSkeletalMesh->SetRelativeScale3D(ItemData->MeshScale);
			ItemSkeletalMesh->SetVisibility(true);

			if (HighlightSkeletalMesh)
			{
				HighlightSkeletalMesh->SetSkeletalMeshAsset(ItemData->PickupSkeletalMesh);
				HighlightSkeletalMesh->SetRelativeScale3D(FVector(1.0f));
			}
		}
		if (ItemMesh) ItemMesh->SetVisibility(false);
	}

	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("UpdateAppearance_BeforeOnItemDataUpdated"));
	OnItemDataUpdated();
	EnsureNameWidgetVisible();
	LogNameWidgetState(TEXT("UpdateAppearance_AfterOnItemDataUpdated"));
}

void AItem::EnsureNameWidgetVisible()
{
	if (!NameWidget)
	{
		return;
	}

	NameWidget->SetVisibility(true, true);
	NameWidget->SetHiddenInGame(false, true);
}

void AItem::LogNameWidgetState(const TCHAR* Context) const
{
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	const UUserWidget* UserWidget = NameWidget ? NameWidget->GetUserWidgetObject() : nullptr;
	const UClass* WidgetClass = NameWidget ? NameWidget->GetWidgetClass() : nullptr;

	FString WidgetItemDataName = TEXT("None");
	FString WidgetDisplayName = TEXT("None");
	if (UserWidget)
	{
		const FObjectProperty* ItemDataProperty = FindFProperty<FObjectProperty>(UserWidget->GetClass(), TEXT("MyItemData"));
		if (!ItemDataProperty)
		{
			ItemDataProperty = FindFProperty<FObjectProperty>(UserWidget->GetClass(), TEXT("ItemData"));
		}

		UObject* WidgetItemDataObject = ItemDataProperty ? ItemDataProperty->GetObjectPropertyValue_InContainer(UserWidget) : nullptr;
		if (const UItemData* WidgetItemData = Cast<UItemData>(WidgetItemDataObject))
		{
			WidgetItemDataName = WidgetItemData->GetName();
			WidgetDisplayName = WidgetItemData->DisplayName.ToString();
		}
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[ItemNameDebug][%s] Actor=%s Class=%s ItemData=%s DisplayName=\"%s\" ItemTag=%s NameWidget=%s WidgetClass=%s UserWidget=%s WidgetItemData=%s WidgetDisplayName=\"%s\" RelLoc=%s RelScale=%s Visible=%s"),
		Context ? Context : TEXT("None"),
		*GetName(),
		*GetClass()->GetName(),
		ItemData ? *ItemData->GetName() : TEXT("None"),
		ItemData ? *ItemData->DisplayName.ToString() : TEXT("None"),
		ItemData ? *ItemData->ItemTag.ToString() : TEXT("None"),
		NameWidget ? TEXT("Valid") : TEXT("None"),
		WidgetClass ? *WidgetClass->GetName() : TEXT("None"),
		UserWidget ? *UserWidget->GetClass()->GetName() : TEXT("None"),
		*WidgetItemDataName,
		*WidgetDisplayName,
		NameWidget ? *NameWidget->GetRelativeLocation().ToString() : TEXT("None"),
		NameWidget ? *NameWidget->GetRelativeScale3D().ToString() : TEXT("None"),
		NameWidget && NameWidget->IsVisible() ? TEXT("true") : TEXT("false"));
}

bool AItem::SnapToGround()
{
	if (!CollisionBox || !GetWorld())
	{
		return false;
	}

	const FVector ActorLoc = GetActorLocation();
	const FVector Start = ActorLoc + FVector(0.f, 0.f, 50.f);
	const FVector End = Start - FVector(0.f, 0.f, FMath::Max(0.f, GroundTraceDistance));

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ItemSnapToGround), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);
	if (!bHit || !Hit.bBlockingHit)
	{
		return false;
	}

	const float HalfHeight = CollisionBox->GetScaledBoxExtent().Z;
	const FVector NewLoc(ActorLoc.X, ActorLoc.Y, Hit.ImpactPoint.Z + HalfHeight);
	SetActorLocation(NewLoc, /*bSweep*/ true);
	return true;
}

void AItem::SetHighlight(bool bEnable, UMaterialInterface* HighlightMaterial)
{

	if (ItemMesh && ItemMesh->IsVisible() && HighlightMesh)
	{
		if (bEnable && HighlightMaterial)
		{
			for (int32 i = 0; i < HighlightMesh->GetNumMaterials(); ++i)
			{
				HighlightMesh->SetMaterial(i, HighlightMaterial);
			}
			HighlightMesh->SetVisibility(true);
		}
		else
		{
			HighlightMesh->SetVisibility(false);
		}
	}

	if (ItemSkeletalMesh && ItemSkeletalMesh->IsVisible() && HighlightSkeletalMesh)
	{
		if (bEnable && HighlightMaterial)
		{
			for (int32 i = 0; i < HighlightSkeletalMesh->GetMaterials().Num(); ++i)
			{
				HighlightSkeletalMesh->SetMaterial(i, HighlightMaterial);
			}
			HighlightSkeletalMesh->SetVisibility(true);
		}
		else
		{
			HighlightSkeletalMesh->SetVisibility(false);
		}
	}
}

void AItem::OnInteract_Implementation()
{
	Destroy();
}
