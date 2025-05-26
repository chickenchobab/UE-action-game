// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Erika.h"
#include "Items/Weapons/RangedWeapon.h"
#include "Items/Weapons/MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"


AErika::AErika()
{
  CombatRadius = 1000.f;
  AcceptanceRadius = 400.f;
  AttackRadius = 700.f;
  SpecialAttackRadius = 200.f;
  CreateBodyWeaponBox(FName("Foot Box"), FName("RightFootSocket"));
}


void AErika::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  
  if (MotionWarping)
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("RotationTarget"), GetRotationWarpTarget());
	}
}


void AErika::BeginPlay()
{
  Super::BeginPlay();

  if (UWorld* World = GetWorld())
  {
    AWeapon* BodyWeapon = Cast<AWeapon>(World->SpawnActor(AWeapon::StaticClass()));
    BodyWeapons.Add(BodyWeapon);
  }
  SetupBodyWeapons(0, 100.f, FName("RightFootSocket"));
}


void AErika::Attack()
{
	Super::Attack();
  if (!CombatTarget) return;

  EnemyState = EEnemyState::EES_Engaged;
  FocusOnTarget();
	if (IsTargetInRange(CombatTarget, SpecialAttackRadius))
	{
    PlaySpecialAttackMontage();
    AttackTime = 0.01f; // Fire soon after kicking
	}
	else
	{
    SpawnProjectile();
    PlayAttackMontage();
    AttackTime = 1.f;
	}
}

bool AErika::CanAttack()
{ 
	return Super::CanAttack() && IsTargetInRange(CombatTarget, AttackRadius);
}


void AErika::CheckCombatTarget()
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
	else if (!IsTargetInRange(CombatTarget, AttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
  else if (IsTargetInRange(CombatTarget, AcceptanceRadius) && !IsTargetInRange(CombatTarget, SpecialAttackRadius) && !IsDetaching())
  {
    if (!IsEngaged() && DetachFromTarget()) // Can step back
    {
      ClearAttackTimer();
      UE_LOG(LogTemp, Warning, TEXT("Detaching"));
    }
    else if (CanAttack()) // Cannot but attack
    {
      StartAttacking(AttackTime);
      UE_LOG(LogTemp, Warning, TEXT("Detaching Attack"));
    }
  }
  else if (CanAttack())
  {
    StartAttacking(AttackTime);
    UE_LOG(LogTemp, Warning, TEXT("Attack"));
  }
}


void AErika::SpawnProjectile()
{
  if (UWorld *World = GetWorld())
  {
    ARangedWeapon *Projectile = World->SpawnActor<ARangedWeapon>(WeaponClass);
    if (Projectile)
    {
      Projectile->Equip(GetMesh(), FName("RightHandSocket"), this, this);
      EquippedWeapon = Cast<AWeapon>(Projectile);
    }
  }
}

void AErika::SetFireTimer()
{
  GetWorldTimerManager().SetTimer(FireTimer, this, &AErika::FireProjectile, 1.f);
}


void AErika::FireProjectile()
{
  if (ARangedWeapon* Projectile = Cast<ARangedWeapon>(EquippedWeapon))
  {
    Projectile->DetachMeshFromSocket();
    if (CombatTarget)
    {
      Projectile->ActivateProjectile(CombatTarget);
    }
    SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
    EquippedWeapon = nullptr;
  }
}
