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

	void PunchOrKick();
	void PunchLeft();
	void PunchRight();
	void Kick();
	void FireEnergyWave();

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* RushMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* LeftPunchMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* RightPunchMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* KickMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* PowerKickMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* ThrowMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TArray<FName> LeftPunchMontageSections;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TArray<FName> RightPunchMontageSections;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TArray<FName> KickMontageSections;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TArray<FName> PowerKickMontageSections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float RushingSpeed = 1000.f;

private:
	FORCEINLINE void ResetCombo() { bInPunchCombo = false; }

	bool bMoveCompleted = false;

	bool bInPunchCombo = false;
	FTimerHandle PunchComboWindowTimer;
	float PunchComboWindow = 1.f;

	float PunchKickRadius = 100.f;
	float EnergyWaveRadius = 700.f;
};