#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class UWidgetComponent;
class UItemData;

UCLASS(Blueprintable, BlueprintType)
class PROJECT_CPR_API AItem : public AActor
{
	GENERATED_BODY()

public:	
	AItem();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

public:
	/** Static Mesh Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HighlightMesh;

	/** Skeletal Mesh Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* ItemSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* HighlightSkeletalMesh;

	/** Interaction Range & Physics Body (Box for stability) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* NameWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|UI")
	float NameVisibleDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	UItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Placement")
	bool bSnapToGroundOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Placement", meta = (ClampMin = "0.0"))
	float GroundTraceDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Placement", meta = (ClampMin = "0.0"))
	float SpawnLiftHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Placement", meta = (ClampMin = "0.0"))
	float PostSpawnSettleDelay = 0.05f;

	UFUNCTION(BlueprintCallable, Category = "Item|Placement")
	bool SnapToGround();

	void InitializeItem(UItemData* NewData);

	void UpdateAppearance();

	void SetHighlight(bool bEnable, UMaterialInterface* HighlightMaterial = nullptr);

	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnItemDataUpdated();

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void OnInteract();
	virtual void OnInteract_Implementation();

private:
	void EnsureNameWidgetVisible();

	void LogNameWidgetState(const TCHAR* Context) const;

	FName AppliedItemTag;
};
