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
  virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const FVector& HitterLocation) override;
  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;

protected:

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarComponent;

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere)
	UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void PerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	virtual void Attack() override;

	virtual int32 PlayDeathMontage() override;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose;

	virtual void Die() override;

	bool IsTargetInRange(AActor* Target, double Radius);

  void CheckCombatTarget();
  void CheckPatrolTarget();

	/** 
	 * Combat
	 */

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 500.f;

	double AttackRadius = 130.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 5.f;


	/**
	 * Navigation
	 */

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite)
	AActor* CurrentPatrolTarget;
	
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	
	void PatrolTimerFinished();
	
	AActor* ChoosePatrolTarget();
	
	void MoveToTarget(AActor* Target);
	
	
private:
	APawn* FindPlayer(const TArray<AActor*>& UpdatedActors);
	
	void ShowHealthBar();
	void HideHealthBar();
	
	void HandleDamage(float DamageAmount);
	
	void GainInterest(AActor* NewTarget);
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	
	FTimerHandle AttackTimer;
	FTimerHandle PatrolTimer;
	
	float AttackTime = 2.f;

	void StartAttackTimer();
	void ClearAttackTimer();
	void ClearPatrolTimer();
};
