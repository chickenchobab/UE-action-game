// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Paladin.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Items/Weapons/Weapon.h"

APaladin::APaladin()
{
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
  }
	else if (!IsTargetInRange(CombatTarget, SpecialAttackRadius) && !IsChasing())
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

void APaladin::SetupShield()
{
	if (Shield)
	{
		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		Shield->AttachToComponent(GetMesh(), TransformRules, FName("LeftHandSocket"));
		Shield->OnComponentBeginOverlap.AddDynamic(this, &APaladin::ShieldBeginOverlap);
	}
}

void APaladin::ShieldBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	// In case the weapon is a projectile
	if (!UKismetSystemLibrary::IsValid(OtherActor)) return;
	
	if (AWeapon* Weapon = Cast<AWeapon>(OtherActor))
	{ 
		if (IsOpposite(Weapon->GetOwner()))
		{
			Weapon->SetBlocked(true);
		}
	}
}

