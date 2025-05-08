// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
	virtual void Jump() override;

  // <IHitInterface>
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	// <\IHitInterface>

	// <IPickupInterface>
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void GetSoul(ASoul* Soul) override;
	virtual void GetGold(ATreasure* Treasure) override;
	// <\IPickupInterface>

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

protected:
	// <AActor>
  virtual void BeginPlay() override;
  // <\AActor>

	// <ABaseCharacter>
	virtual void Attack() override;
	virtual void Die() override;
	virtual bool CanAttack() override;
	virtual void OnAttackEnded() override;
	virtual void OnDodgeEnded() override;
	virtual void HandleDamage(float DamageAmount) override;
	// <\ABaseCharacter>

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EkeyPressed(const FInputActionValue& Value);
	void LeftMouseClicked(const FInputActionValue& Value);
  void Dodge(const FInputActionValue &Value);

  void PlayEquipMontage(const FName &SectionName);
	bool CanDisarm();
	bool CanArm();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();
	UFUNCTION(BlueprintCallable)
	void OnEquipEnded();
	UFUNCTION(BlueprintCallable)
	void OnHitReactEnded();

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SlashCharacterContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EkeyAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

private:
	void EquipWeapon(AWeapon* Weapon);
	void Disarm();
	void Arm();
	void UpdateHealthBar();
	bool CanDodge();
	FORCEINLINE bool IsUnoccupied() { return ActionState == EActionState::EAS_Unoccupied; }

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	FRotator RecentInputRotation = FRotator(0.f, 0.f, 0.f);
};







