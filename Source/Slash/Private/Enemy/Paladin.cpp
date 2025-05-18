// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Paladin.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"

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
		FocusOnTarget();
		PlayAttackMontage();
	}
	else
	{
		PlaySpecialAttackMontage();
	}
}

bool APaladin::CanAttack()
{
	return Super::CanAttack() && IsTargetInRange(CombatTarget, SpecialAttackRadius);
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
		// UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
  }
	else if (!IsTargetInRange(CombatTarget, SpecialAttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
		// UE_LOG(LogTemp, Warning, TEXT("Chase Player"));
	}
	else if (ShouldParry() && !IsParrying())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			Parry();
		}
		// UE_LOG(LogTemp, Warning, TEXT("Parry attack"));
	}
	else if (CanAttack())
	{	
		StartAttacking(IsParrying() ? 0.1f : AttackTime);
		// UE_LOG(LogTemp, Warning, TEXT("Attack Player"));
	}
	else if (IsParrying())
	{
		StopParrying();
		// UE_LOG(LogTemp, Warning, TEXT("Stop parrying"));
	}
}