// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/HitInterface.h"
#include "Enemy.generated.h"

class AAIController;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAnimMontage;
class UMotionWarpingComponent;
class UHealthBarComponent;
class AWeapon;
class ASoul;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	// <AActor>
  virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
	virtual void Destroyed() override;
	// <\AActor>

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter, bool bReact) override;

	FORCEINLINE bool IsChasing() { return EnemyState == EEnemyState::EES_Chasing; }
	FORCEINLINE bool IsAttacking() { return EnemyState == EEnemyState::EES_Attacking; }
	FORCEINLINE bool IsEngaged() { return EnemyState == EEnemyState::EES_Engaged; }
	FORCEINLINE virtual bool IsParrying() override { return EnemyState == EEnemyState::EES_Parrying; }
	FORCEINLINE bool IsDetaching() { return EnemyState == EEnemyState::EES_Detaching; }

protected:

	// <AActor>
  virtual void BeginPlay() override;
  // <\AActor>

	// <ABaseCharacter>
	virtual void Attack() override;
  virtual void Die() override;
	virtual void Parry() override;
  virtual void AttackEnd() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	// <\ABaseCharacter>

	virtual void CheckCombatTarget();
	void CheckPatrolTarget();
	void MoveToTarget(AActor* Target);
	APawn* FindCombatTarget(const TArray<AActor*>& UpdatedActors);
	bool IsTargetInRange(AActor* Target, double Radius);
	void PatrolTimerFinished();
	AActor* ChoosePatrolTarget();
	void ShowHealthBar();
	void HideHealthBar();
	void GainInterest(AActor* NewTarget);
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	void StartAttacking(float AttackDelay);
	void ClearPatrolTimer();
	void ClearAttackTimer();
	void SpawnDefaultWeapon();
	UFUNCTION()
	void PerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	void SpawnSoul();
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();
	bool IsTargetAttacking();
	void StopParrying();
	void FocusOnTarget();
	bool DetachFromTarget();
	void StopDetaching();

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float WarpTargetDistance = 75.f;
	

	/**
	 * Navigation
	 */

	UPROPERTY()
	AAIController* EnemyController;

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere)
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AActor* PatrolTarget;
	
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5.f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrollingSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200.f;


	/** 
	 * Combat
	 */

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarComponent;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DetachingSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 700.f;

	double AcceptanceRadius = 350.f;
	
	double AttackRadius = 100.f;
	double SpecialAttackRadius = 400.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackTime = 1.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 5.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ASoul> SoulClass;

	
	/**
	 * Animation
	 */

	UPROPERTY(VisibleAnywhere)
	UMotionWarpingComponent* MotionWarping;
};