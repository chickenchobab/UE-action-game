// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Erika.generated.h"

class ARangedWeapon;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AErika : public AEnemy
{
	GENERATED_BODY()

public:
	AErika();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
  virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void SpawnProjectile() override;
	virtual void FireProjectile() override;
	virtual void RotateProjectile() override;
	
	virtual void CheckCombatTarget() override;

	void ShootArrow();
	void KickAndDodge();

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* ShootMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* KickAndDodgeMontage;

private:
	float ShootRadius = 700.f;
	float KickRadius = 200.f;
};
