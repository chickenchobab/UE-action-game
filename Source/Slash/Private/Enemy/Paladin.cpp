// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Paladin.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Items/Weapons/Weapon.h"

APaladin::APaladin()
{
	CombatRadius = 700.f;
	AcceptanceRadius = 350.f;
	
	Shield = CreateDefaultSubobject<UStaticMeshComponent>(FName("Shield"));
	Shield->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  Shield->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  Shield->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
}

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
	SetupShield();
}

void APaladin::Attack()
{
	Super::Attack();
	if (!CombatTarget) return;

	EnemyState = EEnemyState::EES_Engaged;
	FocusOnTarget();
	if (IsTargetInRange(CombatTarget, SwordRadius))
	{
		StandingAttack();
	}
	else
	{
		DashAttack();
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
  }
	else if (!IsTargetInRange(CombatTarget, DashAttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
	else if (IsTargetAttacking() && !IsParrying())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			FocusOnTarget();
			Parry();
		}
	}
	else if (CanAttack())
	{	
		StartAttacking(IsParrying() ? 0.1f : AttackTime);
	}
	else if (IsParrying())
	{
		StopParrying();
	}
}

void APaladin::StandingAttack()
{
	PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

void APaladin::DashAttack()
{
	PlayRandomMontageSection(DashAttackMontage, DashAttackMontageSections);
}

void APaladin::SetupShield()
{
	if (Shield)
	{
		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		Shield->AttachToComponent(GetMesh(), TransformRules, FName("LeftHandSocket"));
	}
}