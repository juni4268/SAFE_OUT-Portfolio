#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CPRCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCPRActionComponent;
class AItem;
class UCPRHUDWidget;
class UCPRDieUI;
class UItemData;
class UInputAction;
class ACPREventBase;
class ACPRHardHat;
class ACPRTodoZoneTrigger;

UCLASS()
class PROJECT_CPR_API ACPRCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACPRCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
    ACPREventBase *ActiveEvent;

    UPROPERTY()
    ACPRTodoZoneTrigger *ActiveTodoZone;

    /** Camera Components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USpringArmComponent *SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent *FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actions)
    UCPRActionComponent *ActionComp;

    /** Inventory - 1 Slot */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UItemData *CurrentHeldItem;

    /** Drop Item Actor Class */
    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<AItem> ItemClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCPRHUDWidget> HUDWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    UCPRHUDWidget *HUDWidget;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCPRDieUI> DieUIClass;

    UPROPERTY()
    UCPRDieUI *DieUI;

    void UpdateInteractUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowErrorMessage(FText Message, float DisplayTime = 2.0f);

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bIsWearingHardHat = false;

    UPROPERTY()
    ACPRHardHat *AttachedHardHat;

    void EquipHardHat(UItemData *ItemData);

    void EnableRagdoll();

    bool bIsDead = false;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetCharacterVisibility(bool bVisible, bool bShowUI = true);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extinguisher")
    bool bIsExtinguishing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extinguisher")
    float ExtinguisherRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extinguisher")
    float ExtinguisherRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extinguisher")
    float ExtinguisherPower = 0.1f;

    UFUNCTION(BlueprintImplementableEvent, Category = "Extinguisher")
    void OnExtinguisherStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Extinguisher")
    void OnExtinguisherStop();

    void StartExtinguisher();
    void StopExtinguisher();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase *ChokingSound;

    UPROPERTY()
    UAudioComponent *ChokingAudioComp;

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void OnChokingSound(bool bIsChoking);

protected:
    void UpdateExtinguisher(float DeltaTime);
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
    virtual void Landed(const FHitResult &Hit) override;

    /** Interaction & Inventory */
    void InputActionDrop();
    void InputActionPause(const FInputActionValue &Value);

    /** Input Callbacks */
    void InputActionMove(const FInputActionValue &Value);
    void InputActionLook(const FInputActionValue &Value);
    void InputActionJump(const FInputActionValue &Value);
    void InputActionSprint(const FInputActionValue &Value);
    void InputActionPickUp(const FInputActionValue &Value);
    void InputActionUse(const FInputActionValue &Value);
    void InputActionUseReleased(const FInputActionValue &Value);

private:

    void SetupDeathCamera();
};
