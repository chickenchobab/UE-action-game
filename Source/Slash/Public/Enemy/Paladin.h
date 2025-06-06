// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Paladin.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API APaladin : public AEnemy
{
	GENERATED_BODY()

public:
	APaladin();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	virtual void Attack() override;
	virtual bool CanAttack() override;
	
	virtual void CheckCombatTarget() override;

	void StandingAttack();
	void DashAttack();

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DashAttackMontage;

	UPROPERTY(EditAnywhere)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> DashAttackMontageSections;

private:
	void SetupShield();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Shield;

	float SwordRadius = 100.f;
	float DashAttackRadius = 400.f;
};

