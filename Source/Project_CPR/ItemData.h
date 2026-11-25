#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UTexture2D;
class AItem;

UCLASS(BlueprintType)
class PROJECT_CPR_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UStaticMesh* PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USkeletalMesh* PickupSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FVector MeshScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FVector MeshRelativeLocation = FVector(0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	FVector BoxExtent = FVector(30.0f, 30.0f, 30.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemTag;
};
