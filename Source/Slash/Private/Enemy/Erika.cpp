// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Erika.h"
#include "Items/Weapons/RangedWeapon.h"
#include "Items/Weapons/MeleeWeapon.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"


AErika::AErika()
{
  CombatRadius = 1000.f;
  AcceptanceRadius = 400.f;
  
  CreateHandFootBox(FName("Foot Box"), FName("RightFootSocket"));
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
    HandsAndFeet.Add(BodyWeapon);
  }
  SetupHandFoot(0, 100.f, FName("RightFootSocket"));
}


void AErika::Attack()
{
	Super::Attack();
  if (!CombatTarget) return;

  EnemyState = EEnemyState::EES_Engaged;
  FocusOnTarget();
	if (IsTargetInRange(CombatTarget, KickRadius))
	{
    KickAndDodge();
	}
	else
	{
    ShootArrow();
	}
}

bool AErika::CanAttack()
{ 
	return Super::CanAttack() && IsTargetInRange(CombatTarget, ShootRadius);
}


void AErika::SpawnProjectile()
{
  if (UWorld *World = GetWorld())
  {
    Projectile = World->SpawnActor<ARangedWeapon>(ProjectileClass);
    if (Projectile)
    {
      Projectile->Equip(GetMesh(), FName("RightHandSocket"), this, this);
      Projectile->SetHeadDirection(FVector(0, 0, -1));
      EquippedWeapon = Cast<AWeapon>(Projectile);
    }
  }
}


void AErika::FireProjectile()
{
  if (Projectile)
  {
    Projectile->DetachMeshFromSocket();
    if (CombatTarget)
    {
      RotateProjectile();
      Projectile->ActivateProjectile(CombatTarget);
    }
    SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
    EquippedWeapon = nullptr;
  }
}


void AErika::RotateProjectile()
{
  if (Projectile == nullptr || CombatTarget == nullptr) return;

  FVector RightVector = CombatTarget->GetActorUpVector();
  FVector UpVector = -(CombatTarget->GetActorLocation() - Projectile->GetActorLocation());
  FVector ForwardVector = UpVector.Cross(RightVector);

  FMatrix RotationMatrix(ForwardVector, RightVector, UpVector, FVector::Zero());
  Projectile->SetActorRotation(RotationMatrix.Rotator());
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
	else if (!IsTargetInRange(CombatTarget, ShootRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
  else if (IsTargetInRange(CombatTarget, AcceptanceRadius) && !IsTargetInRange(CombatTarget, KickRadius) && !IsDetaching())
  {
    if (!IsEngaged() && DetachFromTarget()) // Can step back
    {
      ClearAttackTimer();
    }
    else if (CanAttack()) // Cannot but attack
    {
      StartAttacking(AttackTime);
    }
  }
  else if (CanAttack())
  {
    StartAttacking(AttackTime);
  }
}

void AErika::ShootArrow()
{
  PlayMontage(ShootMontage);
  SpawnProjectile();
  AttackTime = 1.f;
}

void AErika::KickAndDodge()
{
	PlayMontage(KickAndDodgeMontage);
  AttackTime = 0.01f; // Fire soon after kicking
}