// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Erika.h"
#include "Items/Weapons/Projectile.h"


AErika::AErika()
{
  CombatRadius = 1000.f;
  AttackRadius = 700.f;
}


void AErika::Attack()
{
	Super::Attack();
  if (!CombatTarget) return;

  EnemyState = EEnemyState::EES_Engaged;
  SpawnProjectile();
  FocusOnTarget();
  PlayAttackMontage();
}

void AErika::Parry()
{
	
}

bool AErika::CanAttack()
{ 
	return Super::CanAttack() && IsTargetInRange(CombatTarget, AttackRadius);
}

void AErika::CheckCombatTarget()
{
  // UE_LOG(LogTemp, Warning, TEXT("Erika Chseck combat target"));
	if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
		// UE_LOG(LogTemp, Warning, TEXT("Erika Lose Interest"));
  }
	else if (!IsTargetInRange(CombatTarget, AttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
		// UE_LOG(LogTemp, Warning, TEXT("Erika Chase Player"));
	}
  else if (CanAttack())
  {
    // UE_LOG(LogTemp, Warning, TEXT("Erika Can attack"));
    StartAttacking(AttackTime);
  }
}

void AErika::SpawnProjectile()
{
  if (UWorld *World = GetWorld())
  {
    AProjectile *Projectile = World->SpawnActor<AProjectile>(WeaponClass);
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
  if (AProjectile* Projectile = Cast<AProjectile>(EquippedWeapon))
  {
    Projectile->DetachMeshFromSocket();
    if (CombatTarget)
    {
      Projectile->ActivateProjectile(CombatTarget);
    }
    EquippedWeapon = nullptr;
  }
}
