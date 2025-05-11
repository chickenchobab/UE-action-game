// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Paladin.h"
#include "MotionWarpingComponent.h"


void APaladin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MotionWarping)
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("TranslationTarget"), GetTranslationWarpTarget());
		MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("RotationTarget"), GetRotationWarpTarget());
	}
}

void APaladin::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultWeapon();
}

void APaladin::Attack()
{
	Super::Attack();
	if (!CombatTarget) return;
	
	EnemyState = EEnemyState::EES_Engaged;
	if (IsTargetInRange(CombatTarget, AttackRadius))
	{
		PlayAttackMontage();
	}
	else
	{
		PlayDashAttackMontage();
	}
}

bool APaladin::CanAttack()
{
	return Super::CanAttack() && IsTargetInRange(CombatTarget, DashAttackRadius);
}

void APaladin::CheckCombatTarget()
{
  if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
		UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
  }
	else if (!IsTargetInRange(CombatTarget, DashAttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
		UE_LOG(LogTemp, Warning, TEXT("Chase Player"));
	}
	else if (CanAttack())
	{
		StartAttacking();	
		UE_LOG(LogTemp, Warning, TEXT("Attack Player"));
	}
}
