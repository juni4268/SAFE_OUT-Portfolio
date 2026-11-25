#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPRHardHat.generated.h"

class UStaticMeshComponent;
class UItemData;

UCLASS()
class PROJECT_CPR_API ACPRHardHat : public AActor
{
	GENERATED_BODY()

public:	
	ACPRHardHat();

	void InitializeHardHat(UItemData* ItemData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HatMesh;
};
