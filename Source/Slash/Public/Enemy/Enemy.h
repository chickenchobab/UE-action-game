// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UAnimMontage;
class UAttributeComponent;
class UHealthBarComponent;
class AAIController;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const FVector& HitterLocation) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarComponent;

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);


	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	void PlayHitReactMontage(const FName& SectionName);

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	void Die();


	bool InTargetRange(AActor* Target, double Radius);

	void CheckCombatTarget();
	void CheckPatrolTarget();

	/** 
	 * Combat
	 */

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	/**
	 * Navigation
	 */

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite)
	AActor* PatrolTarget;
	
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	FTimerHandle PatrolTimer;

	void PatrolTimerFinished();
	
	AActor* ChoosePatrolTarget();

	void MoveToTarget(AActor* Target);


private:
	void DirectionalHitReact(const FVector& ImpactPoint, const FVector& HitterLocation);

};