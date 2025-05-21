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
	
protected:
  virtual void Attack() override;
	virtual bool CanAttack() override;
	
	virtual void CheckCombatTarget() override;
	
	void SpawnProjectile();
	UFUNCTION(BlueprintCallable)
	void SetFireTimer();
	UFUNCTION(BlueprintCallable)
	void FireProjectile();

	// enable foot collision

	FTimerHandle FireTimer;

private:
	FVector SocketLocation;
	FRotator SocketRotation;
};

