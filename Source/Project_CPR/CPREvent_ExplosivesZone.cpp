#include "CPREvent_ExplosivesZone.h"
#include "Components/BoxComponent.h"
#include "Item.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"

ACPREvent_ExplosivesZone::ACPREvent_ExplosivesZone()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    bUseAutoStart = true;
    AutoStartDelay = 0.1f;
    bHasTimeLimit = false;
    bDisableCollisionOnResolve = false;
    bDestroyOnResolve = false;

    ItemDetectionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemDetectionZone"));
    ItemDetectionZone->SetupAttachment(RootComponent);
    ItemDetectionZone->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    ItemDetectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ItemDetectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    ItemDetectionZone->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    ItemDetectionZone->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
}

void ACPREvent_ExplosivesZone::BeginPlay()
{
    Super::BeginPlay();

    if (ItemDetectionZone)
    {
        ItemDetectionZone->OnComponentBeginOverlap.AddDynamic(
            this, &ACPREvent_ExplosivesZone::OnItemZoneBeginOverlap);
        ItemDetectionZone->OnComponentEndOverlap.AddDynamic(
            this, &ACPREvent_ExplosivesZone::OnItemZoneEndOverlap);
    }

    GetWorldTimerManager().SetTimer(InitMaxCountTimerHandle, this, &ACPREvent_ExplosivesZone::InitMaxCount, 0.5f, false);
}

void ACPREvent_ExplosivesZone::InitMaxCount()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundItems;
    UGameplayStatics::GetAllActorsOfClass(World, AItem::StaticClass(), FoundItems);

    int32 TotalExplosivesInMap = 0;
    for (AActor* Actor : FoundItems)
    {
        AItem* Item = Cast<AItem>(Actor);

        if (Item && Item->ItemData)
        {
            if (Item->ItemData->ItemTag == ExplosivesItemTag || Item->ItemData->ItemTag == FName("Explosive"))
            {
                TotalExplosivesInMap++;
            }
        }
    }

    if (TotalExplosivesInMap > 0)
    {
        RequiredCount = TotalExplosivesInMap;
        UE_LOG(LogTemp, Warning, TEXT("[ExplosivesZone] 지연 후 확인: 맵에 배치된 폭발물 %d개를 찾았습니다. 최대치 반영 완료."), RequiredCount);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[ExplosivesZone] 맵에서 %s 태그를 가진 폭발물 아이템을 1개도 찾지 못했습니다! (기본값 %d 유지)"), *ExplosivesItemTag.ToString(), RequiredCount);
    }
}

void ACPREvent_ExplosivesZone::OnItemZoneBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                                                      bool bFromSweep, const FHitResult &SweepResult)
{
    AItem *Item = Cast<AItem>(OtherActor);
    if (!Item || !Item->ItemData)
    {
        return;
    }

    if (Item->ItemData->ItemTag == ExplosivesItemTag)
    {
        RecalculateCount();
    }
}

void ACPREvent_ExplosivesZone::OnItemZoneEndOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                                    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
    AItem *Item = Cast<AItem>(OtherActor);
    if (!Item || !Item->ItemData)
    {
        return;
    }

    if (Item->ItemData->ItemTag == ExplosivesItemTag)
    {
        RecalculateCount();
    }
}

void ACPREvent_ExplosivesZone::RecalculateCount()
{
    if (!ItemDetectionZone)
    {
        return;
    }

    TArray<AActor *> OverlappingActors;
    ItemDetectionZone->GetOverlappingActors(OverlappingActors, AItem::StaticClass());

    int32 NewCount = 0;
    for (AActor *Actor : OverlappingActors)
    {
        AItem *Item = Cast<AItem>(Actor);
        if (Item && Item->ItemData && Item->ItemData->ItemTag == ExplosivesItemTag)
        {
            NewCount++;
        }
    }

    CurrentCount = NewCount;
    ReceiveCountChanged(CurrentCount, RequiredCount);

    if (CurrentCount >= RequiredCount && CurrentStatus != ECPREventStatus::Resolved)
    {
        ResolveEvent();
    }
    else if (CurrentCount < RequiredCount && CurrentStatus == ECPREventStatus::Resolved)
    {
        ResetResolve();
    }
}
