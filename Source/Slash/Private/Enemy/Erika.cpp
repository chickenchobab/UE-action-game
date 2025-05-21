// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Erika.h"
#include "Items/Weapons/RangedWeapon.h"
#include "Components/BoxComponent.h"


AErika::AErika()
{
  BodyBoxes.Add(CreateDefaultSubobject<UBoxComponent>(FName("Foot Box")));
  BodyBoxes[0]->SetupAttachment(GetMesh(), FName("RightFootSocket"));
  BodyBoxes[0]->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
  BodyBoxes[0]->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

  CombatRadius = 1000.f;
  AttackRadius = 700.f;
  SpecialAttackRadius = 200.f;
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
	}
	else
	{
    SpawnProjectile();
    PlayAttackMontage();
	}
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
