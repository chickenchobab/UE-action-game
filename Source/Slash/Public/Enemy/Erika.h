// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Erika.generated.h"

class AProjectile;

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
  virtual void Parry() override;
	virtual bool CanAttack() override;
	
	virtual void CheckCombatTarget() override;
	
	void SpawnProjectile();
	UFUNCTION(BlueprintCallable)
	void SetFireTimer();
	UFUNCTION(BlueprintCallable)
	void FireProjectile();

	FTimerHandle FireTimer;

private:
	FVector SocketLocation;
	FRotator SocketRotation;
};

