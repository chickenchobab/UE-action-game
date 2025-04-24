// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class AAIController;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class AWeapon;
class UAnimMontage;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	// <AActor>
  virtual void Tick(float DeltaTime) override;\
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
	virtual void Destroyed() override;
	// <\AActor>

protected:

	// <AActor>
  virtual void BeginPlay() override;
  // <\AActor>

	// <ABaseCharacter>
	virtual void Attack() override;
	virtual void Die() override;
	virtual void OnAttackEnded() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual int32 PlayDeathMontage() override;
	// <\ABaseCharacter>

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose;
	
private:

	void CheckCombatTarget();
	void CheckPatrolTarget();
	void MoveToTarget(AActor* Target);
	APawn* FindPlayer(const TArray<AActor*>& UpdatedActors);
	bool IsTargetInRange(AActor* Target, double Radius);
	void PatrolTimerFinished();
	AActor* ChoosePatrolTarget();
	void ShowHealthBar();
	void HideHealthBar();
	void GainInterest(AActor* NewTarget);
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	void StartAttacking();
	void ClearPatrolTimer();
	void ClearAttackTimer();
	void SpawnDefaultWeapon();

	FORCEINLINE bool IsChasing() { return EnemyState == EEnemyState::EES_Chasing; }
	FORCEINLINE bool IsAttacking() { return EnemyState == EEnemyState::EES_Attacking; }
	FORCEINLINE bool IsEngaged() { return EnemyState == EEnemyState::EES_Engaged; }

	UFUNCTION()
	void PerceptionUpdated(const TArray<AActor*>& UpdatedActors);

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
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200.f;


	/** 
	 * Combat
	 */
	
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarComponent;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 500.f;
	
	double AttackRadius = 200.f;

	float AttackTime = 2.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 5.f;
};
