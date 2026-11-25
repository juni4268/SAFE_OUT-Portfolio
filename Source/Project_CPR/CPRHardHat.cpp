#include "CPRHardHat.h"
#include "ItemData.h"
#include "Components/StaticMeshComponent.h"

ACPRHardHat::ACPRHardHat()
{
	PrimaryActorTick.bCanEverTick = false;

	HatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatMesh"));
	RootComponent = HatMesh;

	HatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACPRHardHat::InitializeHardHat(UItemData* ItemData)
{
	if (!ItemData || !HatMesh) return;

	if (ItemData->PickupMesh)
	{
		HatMesh->SetStaticMesh(ItemData->PickupMesh);
	}

	HatMesh->SetRelativeScale3D(ItemData->MeshScale);

	HatMesh->SetRelativeLocation(ItemData->MeshRelativeLocation);
}
