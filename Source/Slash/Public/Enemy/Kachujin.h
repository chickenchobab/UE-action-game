// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kachujin.generated.h"

class UAnimMontage;

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
	
	UFUNCTION()
	void ProjectileHit(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	void RushToTarget();
	UFUNCTION(BlueprintCallable)
	void RushStart();
	UFUNCTION()
	void MoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	ARangedWeapon* Throwing;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ARangedWeapon> ThrowingClass;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* RushMontage;

private:
	bool bMoveCompleted = false;
};