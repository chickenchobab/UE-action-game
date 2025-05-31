// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Kachujin.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API AKachujin : public AEnemy
{
	GENERATED_BODY()
	
public:
	AKachujin();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
  virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void SpawnProjectile() override;
	virtual void FireProjectile() override;
	
	virtual void CheckCombatTarget() override;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	ARangedWeapon* Throwing;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ARangedWeapon> ThrowingClass;

};