// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Ninja.generated.h"

class ARangedWeapon;

/**
 * 
 */
UCLASS()
class SLASH_API ANinja : public AEnemy
{
	GENERATED_BODY()

public:
	ANinja();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
  virtual void Attack() override;
	virtual void Dodge() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	virtual void SpawnProjectile() override;
	virtual void FireProjectile() override;
	virtual void RotateProjectile(ARangedWeapon* Projectile) override;
	
	virtual void CheckCombatTarget() override;

	void SpawnDefaultWeapon();
	
	void BackAttack();
	void PlayBackAttackMontage();
	UFUNCTION(BlueprintCallable)
	void KickStart();

	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	ARangedWeapon* Throwing;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ARangedWeapon> ThrowingClass;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* BackAttackMontage;

private:
	void EnableCollision();
	void DisableCollision();
	void EnableWeaponHitReaction(bool bEnabled);

	// Asset-specific functions
	void ReverseWeaponMesh();
	void RestoreWeaponMesh();
};


