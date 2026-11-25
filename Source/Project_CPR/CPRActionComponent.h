#pragma once

#include "CPREventBase.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "ItemData.h"
#include "CPRActionComponent.generated.h"

class ACPRCharacter;
class UInputMappingContext;
class UInputAction;
class AItem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_CPR_API UCPRActionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCPRActionComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext *DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *PickUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *UseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *DropAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction *PauseAction;

    void CheckForItems();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundBase *PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundBase *DropSound;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    UMaterialInterface *ItemHighlightMaterial;

    UPROPERTY()
    AItem *CurrentTargetItem;

    void ActionPlayerMove(FVector2D MoveDirection2D);
    void ActionLookAndTurn(FVector2D LookTurnDirection2D);
    void ActionJump(bool IsJump);
    void ActionSprint(bool IsSprint);
    void ActionPickUp();
    void ActionUse();
    bool ActionDrop();
    void ActionPause(bool IsPause);

private:
    UPROPERTY()
    ACPRCharacter *Player;

    float WalkSpeed = 600.f;
    float SprintSpeed = 1200.f;
};
